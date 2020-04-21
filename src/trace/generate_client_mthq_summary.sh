#!/bin/bash

babeltrace2 ust/ > mthq_trace.txt

curr_time=$(date "+%Y%m%d%H%M%S")
summary_file_name=$1
summary_file_name="$summary_file_name"_"$curr_time"

dash_mode=`cat mthq_trace.txt | grep "dash_mode_field" | awk -F " " '{print $13}'`
dash_mode=$(echo $dash_mode | awk -F "\"" '{print $2}')

seg_dur=`cat mthq_trace.txt | grep "segment_duration_field" | awk -F " " '{print $16}'`
seg_dur=$(echo $seg_dur | awk -F "," '{print $1}')

total_dur=`cat mthq_trace.txt | grep "total_duration_field" | awk -F " " '{print $19}'`
total_dur=$(echo $total_dur | awk -F "," '{print $1}')
total_dur=`awk 'BEGIN{printf "%.2f\n", '$total_dur/1000'}'`

# seg_num=`expr $total_dur / $seg_dur / 1000`
#tmp_seg_num=`awk 'BEGIN{printf "%.2f\n", '$total_dur/$seg_dur'}'`
#seg_num=$((${tmp_seg_num//.*/+1}))
abs() { echo ${1#-};}
tmp_seg_num=`awk 'BEGIN{printf "%.2f\n", '$total_dur/$seg_dur'}'`
tmp_seg_num_arr=(${tmp_seg_num//./ })
tmp_seg_num_int=${tmp_seg_num_arr[0]}
diff=`awk 'BEGIN{printf "%.2f\n", '$tmp_seg_num-$tmp_seg_num_int'}'`
diff=`abs $diff`
if [ `echo "$diff < 0.01" | bc` -eq 1 ]
then
    seg_num=$tmp_seg_num_int
else
    seg_num=$(($tmp_seg_num_int+1))
fi

frame_rate=`cat mthq_trace.txt | grep "frame_rate_field" | awk -F " " '{print $22}'`
frame_rate=$(echo $frame_rate | awk -F "," '{print $1}')

frame_num=`cat mthq_trace.txt | grep "frame_num_field" | awk -F " " '{print $25}'`
frame_num=$(echo $frame_num | awk -F "," '{print $1}')


width=`cat mthq_trace.txt | grep "highreso_width_field" | awk -F " " '{print $28}'`
width=$(echo $width | awk -F "," '{print $1}')

height=`cat mthq_trace.txt | grep "highreso_height_field" | awk -F " " '{print $31}'`
height=$(echo $height | awk -F "," '{print $1}')

echo "-------------------------Motion to high quality--------------------------------------" >> "$summary_file_name".txt
echo "DASH mode                              : $dash_mode" >> "$summary_file_name".txt
echo "Resolution                             : $width x $height" >> "$summary_file_name".txt
echo "Segment duration                       : $seg_dur second" >> "$summary_file_name".txt
echo "Total presentation duration            : $total_dur seconds" >> "$summary_file_name".txt
echo "Media stream frame rate                : $frame_rate" >> "$summary_file_name".txt
echo "Total frames number                    : $frame_num" >> "$summary_file_name".txt

# calc local time to totalTime
calc_time()
{
    form=$1
    Time=(${form//./ })
    onlySec=`date -d ${Time[0]} +%s`
    onlyNaSec=`awk 'BEGIN{printf "%.9f\n", '${Time[1]}/1000000000'}'`
    totalTime=`awk 'BEGIN{printf "%.9f\n", '$onlySec+$onlyNaSec'}'`
    echo $totalTime
}
decode_fifo_num=`cat mthq_trace.txt | grep  "decode_fifo" | awk -F " " '{print $13}'`

# filter init pose change logs
start=`cat mthq_trace.txt | grep -n "is_changed_field = 1" | awk -F ":" '{print $1}'`
start=`echo $start | awk -F " " '{print $1}'`
# get timeArray from T2 - T9 in every segment && isChangedArray && segIDArray
cur_index=$start

for i in $(seq 1 $seg_num)
do
    index=0
    T2=`cat mthq_trace.txt | awk "NR > $cur_index" | grep -m 1 "T2_detect_pose_change" | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
    eval timeArraySeg$i[$(($index))]=$T2
    # echo "T2:$T2"
    timeT2=`calc_time $T2`

    is_changed=`cat mthq_trace.txt | awk "NR > $cur_index" | grep -m 1 "T2_detect_pose_change" | awk -F " " '{print $13}'`
    isChangedArray[$i]=$is_changed
    # echo "is_changed:$is_changed"

    T3=`cat mthq_trace.txt | awk "NR > $cur_index" | grep -m 1 "T3_start_download_time" | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
    eval timeArraySeg$i[$(($index+1))]=$T3
    # echo "T3:$T3"
    timeT3=`calc_time $T3`
    timeAverageArray[0]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[0]}+$timeT3-$timeT2'}'`

    segID=`cat mthq_trace.txt | awk "NR > $cur_index" | grep -m 1 "T3_start_download_time" | awk -F " " '{print $13}'`
    segIDArray[$i]=$segID
    # echo "segID:$segID"

    T4=`cat mthq_trace.txt | awk "NR > $cur_index" | grep -m 1 "T4_parse_start_time" | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
    eval timeArraySeg$i[$(($index+2))]=$T4
    # echo "T4:$T4"
    timeT4=`calc_time $T4`
    timeAverageArray[1]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[1]}+$timeT4-$timeT3'}'`

    T5=`cat mthq_trace.txt | awk "NR > $cur_index" | grep -m 1 "T5_read_start_time" | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
    eval timeArraySeg$i[$(($index+3))]=$T5
    # echo "T5:$T5"
    timeT5=`calc_time $T5`
    timeAverageArray[2]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[2]}+$timeT5-$timeT4'}'`

    T6=`cat mthq_trace.txt | awk "NR > $cur_index" | grep -m 1 "T6_read_end_time" | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
    eval timeArraySeg$i[$(($index+4))]=$T6
    # echo "T6:$T6"
    timeT6=`calc_time $T6`
    timeAverageArray[3]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[3]}+$timeT6-$timeT5'}'`

    T7=`cat mthq_trace.txt | awk "NR > $cur_index" | grep "T7_get_packet" | grep -m 1 "segment_id_field = $segID " | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
    eval timeArraySeg$i[$(($index+5))]=$T7
    # echo "T7:$T7"
    timeT7=`calc_time $T7`
    T7_row=`cat mthq_trace.txt | grep -n $T7 | awk -F ":" '{print $1}'`
    timeAverageArray[4]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[4]}+$timeT7-$timeT6'}'`

    T8=`cat mthq_trace.txt | awk "NR > $T7_row" | grep -m 1 "T8_decode_finish" | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
    eval timeArraySeg$i[$(($index+6))]=$T8
    # echo "T8:$T8"
    timeT8=`calc_time $T8`
    timeAverageArray[5]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[5]}+$timeT8-$timeT7'}'`
    fifo_num=`cat mthq_trace.txt | awk "NR > $T7_row" | grep -m 1 "T8_decode_finish" | awk -F " " '{print $13}'`
    # echo "fifo_num:$fifo_num"
    T8_row=`cat mthq_trace.txt | grep -n $T8 | awk -F ":" '{print $1}'`
    waiting_times=`expr $decode_fifo_num + $fifo_num - 1`
    # echo "waiting_times:$waiting_times"

    T9=`cat mthq_trace.txt | awk "NR > $T8_row" | grep -m $waiting_times "T9_render" | tail -n 1 | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
    eval timeArraySeg$i[$(($index+7))]=$T9
    # echo "T9:$T9"
    timeT9=`calc_time $T9`
    timeAverageArray[6]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[6]}+$timeT9-$timeT8'}'`
    cur_index=`cat mthq_trace.txt | grep -n $T6 | awk -F ":" '{print $1}'`
    # echo "cur_index:$cur_index"
done
# calc average time for each part
for j in $(seq 0 $((${#timeAverageArray[@]}-1)))
do
    timeAverageArray[$j]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[$j]}/$seg_num'}'`
done

# echo ${timeAverageArray[@]}

for ii in $(seq 1 $seg_num)
do
    eval total_T2toT9=(${total_T2toT9[@]} '$'{timeArraySeg${ii}[@]})
done


# get [T1,T9n]s
T1=`cat mthq_trace.txt | grep "T1_change_to_lowQ" | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
T1Array=(${T1// / })
T9n=`cat mthq_trace.txt | grep "T9n_change_to_highQ" | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
T9nArray=(${T9n// / })
changedTimes=${#T9nArray[@]}
if [ $changedTimes -eq 0 ]
then
    echo "---------------There is no motion in this test!----------------" >> "$summary_file_name".txt
else
# echo "changeTime$changedTimes"
# calc total average time interval from T1 to T9n
for i in $(seq 0 $(($changedTimes-1)))
do
    timeT9n=`calc_time ${T9nArray[$i]}`
    timeT1=`calc_time ${T1Array[$i]}`
    changedAVGTime=`awk 'BEGIN{printf "%.9f\n", '$changedAVGTime+$timeT9n-$timeT1'}'`
done
changedAVGTime=`awk 'BEGIN{printf "%.9f\n", '$changedAVGTime/$changedTimes'}'`

# get [T2,T9] in [T1,T9n]s and echo to file
cur_T1=0
T1_time=`calc_time ${T1Array[$cur_T1]}`
T9n_time=`calc_time ${T9nArray[$cur_T1]}`

echo "-------------------[Time Cost] Motion $(($cur_T1+1)) happens and FOV start to render low quality tiles-------------------" >> "$summary_file_name".txt
echo "(T1) change to low quality                        - player :    ${T1Array[0]}" >> "$summary_file_name".txt
in_flag=0 # if [T2,T9] in [T1,T9']
is_quit_finding=0 # if quiting finding [T2,T9] in [T1,T9']
nochangedTimes=0 # pose change but FOV quality remains high quality
detectInChanged=0
for iter in $(seq 0 $((${#total_T2toT9[@]}-1)))
do
    # 1.calc elem time in totalArray
    elem_time=`calc_time ${total_T2toT9[$iter]}`
    # 2.if last T1-T9 has been echoed.
    if [ $in_flag -eq 1 ] && [ `echo "$elem_time > $T9n_time + 0.04" | bc` -eq 1 ]
    then
        echo "(T9') blit all high quality tiles in FOV  - OmafDashAccess :    ${T9nArray[$cur_T1]}" >> "$summary_file_name".txt
        # echo T9n-T1 time interval
        timeCost=`awk 'BEGIN{printf "%.9f\n", '$T9n_time-$T1_time'}'`
        echo "Motion to high quality time cost is                                    $timeCost" >> "$summary_file_name".txt
        cur_T1=$(($cur_T1+1))
        # already finished
        if [ $cur_T1 -eq $changedTimes ]
        then
            is_quit_finding=1
        fi
        in_flag=0
        if [ $is_quit_finding -eq 0 ]
        then
            echo "-------------------[Time Cost] Motion $(($cur_T1+1)) happens and FOV start to render low quality tiles-------------------" >> "$summary_file_name".txt
            echo "(T1) change to low quality                        - player :    ${T1Array[$cur_T1]}" >> "$summary_file_name".txt
            # 3. update [T1,T9]
            T1_time=`calc_time ${T1Array[$cur_T1]}`
            T9n_time=`calc_time ${T9nArray[$cur_T1]}`
        fi
    fi
    # 4. start to echo which satisfies the condition
    if [ $is_quit_finding -eq 0 ] && [ `echo "$elem_time > $T1_time" | bc` -eq 1 ] && [ `echo "$elem_time < $T9n_time + 0.04"  | bc` -eq 1 ]
    then
        in_flag=1
        category=`expr $iter % 8`
        segID=`expr $iter / 8 + 1`
        segID=${segIDArray[$segID]}
        if [ $category -eq 0 ]
        then
            echo "(T2) pose changed                         - OmafDashAccess :    ${total_T2toT9[$iter]}" >> "$summary_file_name".txt
            detectTimeAverage=`awk 'BEGIN{printf "%.9f\n", '$detectTimeAverage+$elem_time-$T1_time'}'`
	    detectInChanged=$(($detectInChanged+1))
        elif [ $category -eq 1 ]
        then
            echo "(T3) start to download segment $segID          - OmafDashAccess :    ${total_T2toT9[$iter]}" >> "$summary_file_name".txt
        elif [ $category -eq 2 ]
        then
            echo "(T4) start to parse segment $segID             - OmafDashAccess :    ${total_T2toT9[$iter]}" >> "$summary_file_name".txt
        elif [ $category -eq 3 ]
        then
            echo "(T5) start to read segment $segID              - OmafDashAccess :    ${total_T2toT9[$iter]}" >> "$summary_file_name".txt
        elif [ $category -eq 4 ]
        then
            echo "(T6) segment $segID has been read              - OmafDashAccess :    ${total_T2toT9[$iter]}" >> "$summary_file_name".txt
        elif [ $category -eq 5 ]
        then
            echo "(T7) get first packet from segment $segID      - OmafDashAccess :    ${total_T2toT9[$iter]}" >> "$summary_file_name".txt
        elif [ $category -eq 6 ]
        then
            echo "(T8) The next decode finished             - OmafDashAccess :    ${total_T2toT9[$iter]}" >> "$summary_file_name".txt
        elif [ $category -eq 7 ]
        then
            echo "(T9) render first packet in segment $segID             - player :    ${total_T2toT9[$iter]}" >> "$summary_file_name".txt
        fi
    else
        # not in [T1,T9'] but pose changed
        category=`expr $iter % 8`
        seg_id=`expr $iter / 8 + 1` # 1 - seg_num
        # ensure [T2,T9] not in [T1,T9']
        if [ $category -eq 0 ] && [ ${isChangedArray[$seg_id]} -eq 1 ]
        then
            cur_T9=`calc_time ${total_T2toT9[$(($iter+7))]}`
            if [ `echo "$cur_T9 < $T1_time" | bc` -eq 1 ]
            then
                nochangedTimes=$(($nochangedTimes+1))
                # echo "seg_id$seg_id"
            fi
        fi
    fi
done
# # 5.echo the last T9' information if occurs in last segment
if [ `echo "$T9n_time + 0.4 > $elem_time" | bc` -eq 1 ]
then
    echo "(T9') blit all high quality tiles in FOV  - OmafDashAccess :    ${T9nArray[$cur_T1]}" >> "$summary_file_name".txt
    # echo T9n-T1 time interval
    timeCost=`awk 'BEGIN{printf "%.9f\n", '$T9n_time-$T1_time'}'`
    echo "Motion to high quality time cost is                                    $timeCost" >> "$summary_file_name".txt
fi

echo "--------------------------------[Average Time Cost]--------------------------------" >> "$summary_file_name".txt
echo "Motion changes $(($changedTimes+$nochangedTimes)) times, in which $changedTimes of them occurs low quality in FOV" >> "$summary_file_name".txt
echo "Motion from low quality to high quality average time cost  :  $changedAVGTime s" >> "$summary_file_name".txt
totalAVGTime=`awk 'BEGIN{printf "%.9f\n", '$changedAVGTime\*$changedTimes/$(($changedTimes+$nochangedTimes))'}'`
echo "Total motion to high quality average time cost             :  $totalAVGTime s" >> "$summary_file_name".txt
echo "--------------------------------[Average Time Cost in sub modules]--------------------------------" >> "$summary_file_name".txt
if [ $detectInChanged -eq 0 ]
then
    detectTimeAverage=0
else
    detectTimeAverage=`awk 'BEGIN{printf "%.9f\n", '$detectTimeAverage/$detectInChanged'}'`
fi
echo "Detect pose change time cost                               :  $detectTimeAverage s" >> "$summary_file_name".txt
echo "Download segment time cost                                 :  ${timeAverageArray[1]} s" >> "$summary_file_name".txt
echo "Parse segment time cost                                    :  ${timeAverageArray[2]} s" >> "$summary_file_name".txt
echo "Read segment time cost                                     :  ${timeAverageArray[3]} s" >> "$summary_file_name".txt
# echo "Wait for player getting first packet in segment time cost  :  ${timeAverageArray[4]} s" >> "$summary_file_name".txt
echo "Decode one frame time cost                                 :  ${timeAverageArray[5]} s" >> "$summary_file_name".txt
echo "Wait for first packet in segment to render time cost       :  ${timeAverageArray[6]} s" >> "$summary_file_name".txt
fi
