#!/bin/bash

babeltrace2 ust/ > mthq_trace.txt

curr_time=$(date "+%Y%m%d%H%M%S")
summary_file_name=$1
summary_file_name="$summary_file_name"_"$curr_time"

dash_mode=`cat mthq_trace.txt | grep "dash_mode_field" | awk -F " " '{print $13}'`
dash_mode=$(echo $dash_mode | awk -F "\"" '{print $2}')

proj_format=`cat mthq_trace.txt | grep "projection_format_field" | awk -F " " '{print $16}'`
proj_format=$(echo $proj_format | awk -F "," '{print $1}')
if [ $proj_format -eq 0 ]
then
    proj_format="ERP"
elif [ $proj_format -eq 1 ]
then
    proj_format="Cubemap"
else
    echo "The projection format is not supported!"
    exit 1
fi

seg_dur=`cat mthq_trace.txt | grep "segment_duration_field" | awk -F " " '{print $19}'`
seg_dur=$(echo $seg_dur | awk -F "," '{print $1}')

total_dur=`cat mthq_trace.txt | grep "total_duration_field" | awk -F " " '{print $22}'`
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

frame_rate=`cat mthq_trace.txt | grep "frame_rate_field" | awk -F " " '{print $25}'`
frame_rate=$(echo $frame_rate | awk -F "," '{print $1}')

frame_num=`cat mthq_trace.txt | grep "frame_num_field" | awk -F " " '{print $28}'`
frame_num=$(echo $frame_num | awk -F "," '{print $1}')


width=`cat mthq_trace.txt | grep "highreso_width_field" | awk -F " " '{print $31}'`
width=$(echo $width | awk -F "," '{print $1}')

height=`cat mthq_trace.txt | grep "highreso_height_field" | awk -F " " '{print $34}'`
height=$(echo $height | awk -F "," '{print $1}')

packing_method=`cat mthq_trace.txt | grep "track_type_field" | awk -F " " '{print $13}'`
packing_method=$(echo $packing_method | awk -F "\"" '{print $2}')
if [ "$packing_method" == "tiletracks" ]
then
	packing_method="later binding"
elif [ "$packing_method" == "extractortrack" ]
then
	packing_method="extractor track"
else
	echo "The packing method is not supported!"
	exit 1
fi


echo "----------------------------Motion to high quality---------------------------------------------------------" >> "$summary_file_name".txt
echo "DASH mode                              : $dash_mode" >> "$summary_file_name".txt
echo "Projection format                      : $proj_format" >> "$summary_file_name".txt
echo "Pakcing method                         : $packing_method" >> "$summary_file_name".txt
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
#decode_fifo_num=`cat mthq_trace.txt | grep  "decode_fifo" | awk -F " " '{print $13}'`

# filter init pose change logs
start=`cat mthq_trace.txt | grep -n -m 1 "is_changed_field = 1" | awk -F ":" '{print $1}'`
start=`echo $start | awk -F " " '{print $1}'`
cur_index=$(($start-1))

motion_change_times=0
select_times=0

# fill the value that not needed
valueNotAvailable="00:00:00.000000000"

# 1. get timeArraySegi[0,1,2,3]:timeline && timeAverageArray[i]:timeinterval
for i in $(seq 1 $seg_num)
do
    index=0
    #------------------T2-pose changed------is_changed-------------#
    T2=`cat mthq_trace.txt | awk "NR > $cur_index" | grep -m 1 "T2_detect_pose_change" | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
    eval timeArraySeg$i[$(($index+1))]=$T2
    # echo "T2:$T2"
    timeT2=`calc_time $T2`
    is_changed=`cat mthq_trace.txt | awk "NR > $cur_index" | grep -m 1 "T2_detect_pose_change" | awk -F " " '{print $13}'`
	if [ $is_changed -eq 1 ]
    then
        motion_change_times=$motion_change_times+1
    fi
    # echo "is_changed:$is_changed"
    #------------------T1-select tracks-----------------------------#
    cur_index=`cat mthq_trace.txt | grep -n $T2 | awk -F ":" '{print $1}'`
    T1=`cat mthq_trace.txt | awk "NR < $cur_index" | grep -n "T1_select_tracks" | tail -n 1 | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
    timeT1=`calc_time $T1`
    timeInter=`awk 'BEGIN{printf "%.9f\n", '$timeT2-$timeT1'}'` # find out a T1 that is upper than T2 within a very short time interval
    if [ `echo "$timeInter < 0.5" | bc` -eq 1 ]
    then
        eval timeArraySeg$i[$(($index))]=$T1
        select_times=$(($select_times+1))
        timeAverageArray[0]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[0]}+$timeT2-$timeT1'}'`
    else
        eval timeArraySeg$i[$(($index))]=$valueNotAvailable
    fi
    #------------------T3-download start------segid------------------#
    T3=`cat mthq_trace.txt | awk "NR > $cur_index" | grep -m 1 "T3_start_download_time" | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
    eval timeArraySeg$i[$(($index+2))]=$T3
    # echo "T3:$T3"
    timeT3=`calc_time $T3`
    timeAverageArray[1]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[1]}+$timeT3-$timeT2'}'`
    # find out the segment id
    segID=`cat mthq_trace.txt | awk "NR > $cur_index" | grep -m 1 "T3_start_download_time" | awk -F " " '{print $13}'`
    segIDArray[$i]=$segID
    # echo "segID:$segID"
    #------------------T4-parse start-----------------------------#
    T4=`cat mthq_trace.txt | awk "NR > $cur_index" | grep -m 1 "T4_parse_start_time" | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
    eval timeArraySeg$i[$(($index+3))]=$T4
    # echo "T4:$T4"
    timeT4=`calc_time $T4`
    timeAverageArray[2]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[2]}+$timeT4-$timeT3'}'`
    #------------------T5-parse end find the last T5 in segid----#
    T5=`cat mthq_trace.txt | awk "NR > $cur_index" | grep "T5_parse_end_time" | grep "segment_id_field = $segID " | tail -n 1 | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
    eval timeArraySeg$i[$(($index+4))]=$T5
    # echo "T5:$T5"
    timeT5=`calc_time $T5`
    timeAverageArray[3]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[3]}+$timeT5-$timeT4'}'`
    #------------------T6-stitch start(only in later binding)-#
    if [ "$packing_method" == "later binding" ]
    then
        T6=`cat mthq_trace.txt | awk "NR > $cur_index" | grep -m 1 "T6_stitch_start_time" | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
        eval timeArraySeg$i[$(($index+5))]=$T6
        # echo "T6:$T6"
        timeT6=`calc_time $T6`
        timeAverageArray[4]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[4]}+$timeT6-$timeT5'}'`
    else
        eval timeArraySeg$i[$(($index+5))]=$valueNotAvailable
        timeAverageArray[4]=0
    fi
    #------------------T7-stitch end(only in later binding)--segment_id, pts, video_num---#
    if [ "$packing_method" == "later binding" ]
    then
        T7=`cat mthq_trace.txt | awk "NR > $cur_index" | grep "T7_stitch_end_time" | grep -m 1 "segment_id_field = $segID," | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
        eval timeArraySeg$i[$(($index+6))]=$T7
        timeT7=`calc_time $T7`
        # obtain pts information in later binding
        pts=`cat mthq_trace.txt | awk "NR > $cur_index" | grep "T7_stitch_end_time" | grep -m 1 "segment_id_field = $segID," | awk -F " " '{print $16}' | awk -F "," '{print $1}'`
        ptsArray[$i]=$pts
        video_num=`cat mthq_trace.txt | awk "NR > $cur_index" | grep "T7_stitch_end_time" | grep -m 1 "segment_id_field = $segID," | awk -F " " '{print $19}'`
        videoNumArray[$i]=$video_num
        timeAverageArray[5]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[5]}+$timeT7-$timeT6'}'`
    else
        eval timeArraySeg$i[$(($index+6))]=$valueNotAvailable
        video_num=1
        videoNumArray[$i]=$video_num
        timeAverageArray[5]=0
    fi
    #------------------T8-get packet in player pts----------------#
    cur_index=`cat mthq_trace.txt | grep -n $T5 | awk -F ":" '{print $1}'`
    T8=`cat mthq_trace.txt | awk "NR > $cur_index" | grep -m 1 "T8_get_packet" | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
    eval timeArraySeg$i[$(($index+7))]=$T8
    # echo "T8:$T8"
    timeT8=`calc_time $T8`
    if [ "$packing_method" == "later binding" ]
    then
        timeAverageArray[6]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[6]}+$timeT8-$timeT7'}'`
    else
        #obtain pts information in extractor track
        pts=`cat mthq_trace.txt | awk "NR > $cur_index" | grep -m 1 "T8_get_packet" | awk -F " " '{print $13}' | awk -F "," '{print $1}'`
        ptsArray[$i]=$pts
        timeAverageArray[6]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[6]}+$timeT8-$timeT5'}'`
    fi
    max_video_num=5
    #------------------T9-push one frame at pts at video_id----------------#
    maxTimeT9=0
    for video_id in $(seq 1 $video_num)
    do
        T9=`cat mthq_trace.txt | awk "NR > $cur_index" | grep "T9_push_frame_to_fifo" | grep "pts_field = $pts," | grep -m 1 "video_id_field = $(($video_id -1)) " | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
        eval timeArraySeg$i[$(($index+8+$video_id-1))]=$T9
        # echo "T9:$T9"
        timeT9=`calc_time $T9`
		if [ `echo "$timeT9 > $maxTimeT9" | bc` -eq 1 ]
		then
			maxTimeT9=$timeT9
		fi
    done
    for video_id in $(seq $(($video_num+1)) $max_video_num)
    do
        eval timeArraySeg$i[$(($index+8+$video_id-1))]=$valueNotAvailable
    done
    timeAverageArray[7]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[7]}+$maxTimeT9-$timeT8'}'`
    #------------------T10 only some data-----------------------------------#
    #------------------T11-update one frame at pts--------------------------#
	T11=`cat mthq_trace.txt | awk "NR > $cur_index" | grep "T11_update_time" | grep -m 1 "pts_field = $pts " | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
    eval timeArraySeg$i[$(($index+8+$max_video_num))]=$T11
    # echo "T8:$T8"
    timeT11=`calc_time $T11`
    timeAverageArray[8]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[8]}+$timeT11-$maxTimeT9'}'`
    #------------------T12-change to high quality at pts--------------------------#
    #------------------T13-render time at pts-------------------------------------#
	T13=`cat mthq_trace.txt | awk "NR > $cur_index" | grep "T13_render_time" | grep -m 1 "render_count_field = $pts " | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
    eval timeArraySeg$i[$(($index+9+$max_video_num))]=$T13
    # echo "T8:$T8"
    timeT13=`calc_time $T13`
    if [ $pts -gt 0 ]
    then
    timeAverageArray[9]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[9]}+$timeT13-$timeT11'}'`
    fi
done

# 2. calc average time for each part
# echo "select time : $select_times"
timeAverageArray[0]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[0]}/$select_times'}'`
for j in $(seq 1 $((${#timeAverageArray[@]}-1)))
do
    timeAverageArray[$j]=`awk 'BEGIN{printf "%.9f\n", '${timeAverageArray[$j]}/$seg_num'}'`
done

# echo ${timeAverageArray[@]}

# 3. serialize timeline of each part
for ii in $(seq 1 $seg_num)
do
    eval total_T1toT13=(${total_T1toT13[@]} '$'{timeArraySeg${ii}[@]})
done


# 4. get [T0,T12]s and pts
T0=`cat mthq_trace.txt | grep "T0_change_to_lowQ" | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
T0Array=(${T0// / })
T12=`cat mthq_trace.txt | grep "T12_change_to_highQ" | awk -F " " '{print $1}' | awk -F "[][]" '{print $2}'`
T12Array=(${T12// / })

T0pts=`cat mthq_trace.txt | grep "T0_change_to_lowQ" | awk -F " " '{print $16}'`
T0ptsArrary=(${T0pts// / })

T12pts=`cat mthq_trace.txt | grep "T12_change_to_highQ" | awk -F " " '{print $16}'`
T12ptsArrary=(${T12pts// / })

# 5. extract timeline in [T0, T12]s
changedTimes=${#T12Array[@]}
if [ $changedTimes -eq 0 ]
then
    echo "---------------There is no motion in this test!----------------" >> "$summary_file_name".txt
else
# echo "changeTime$changedTimes"
# 5.1 calc total average time interval from T1 to T9n
for i in $(seq 0 $(($changedTimes-1)))
do
    timeT12=`calc_time ${T12Array[$i]}`
    timeT0=`calc_time ${T0Array[$i]}`
    changedAVGTime=`awk 'BEGIN{printf "%.9f\n", '$changedAVGTime+$timeT12-$timeT0'}'`
done
changedAVGTime=`awk 'BEGIN{printf "%.9f\n", '$changedAVGTime/$changedTimes'}'`

# 5.2 get [T2,T9] in [T1,T9n]s and echo to file
cur_T0=0
T0_time=`calc_time ${T0Array[$cur_T0]}`
T12_time=`calc_time ${T12Array[$cur_T0]}`

echo "-------------------[Time Cost] Motion $(($cur_T0+1)) happens and FOV start to render low quality tiles-------------------" >> "$summary_file_name".txt
echo "(T0) change to low quality at pts ${T0ptsArrary[0]}              - player :    ${T0Array[0]}" >> "$summary_file_name".txt
in_flag=0 # if [T2,T9] in [T1,T9']
is_quit_finding=0 # if quiting finding [T2,T9] in [T1,T9']
detectInChanged=0
#total_T1toT13 have max 15 elements.
for iter in $(seq 0 $((${#total_T1toT13[@]}-1)))
do
    # 1.calc elem time in totalArray
    elem_time=`calc_time ${total_T1toT13[$iter]}`
    # 2.if last T1-T12 has been echoed.
    if [ $in_flag -eq 1 ] && [ `echo "$elem_time > $T12_time + 0.01" | bc` -eq 1 ]
    then
        echo "(T12) blit all high quality tiles in FOV at pts ${T12ptsArrary[$cur_T0]} - player :    ${T12Array[$cur_T0]}" >> "$summary_file_name".txt
        # echo T12-T1 time interval
        timeCost=`awk 'BEGIN{printf "%.9f\n", '$T12_time-$T0_time'}'`
        echo "Motion to high quality time cost is                                    $timeCost" >> "$summary_file_name".txt
        cur_T0=$(($cur_T0+1))
        # already finished
        if [ $cur_T0 -eq $changedTimes ]
        then
            is_quit_finding=1
        fi
        in_flag=0
        if [ $is_quit_finding -eq 0 ]
        then
            echo "-------------------[Time Cost] Motion $(($cur_T0+1)) happens and FOV start to render low quality tiles-------------------" >> "$summary_file_name".txt
            echo "(T0) change to low quality at pts ${T0ptsArrary[$cur_T0]}             - player :    ${T0Array[$cur_T0]}" >> "$summary_file_name".txt
            # 3. update [T1,T12]
            T0_time=`calc_time ${T0Array[$cur_T0]}`
            T12_time=`calc_time ${T12Array[$cur_T0]}`
        fi
    fi
    # 4. start to echo which satisfies the condition
    if [ $is_quit_finding -eq 0 ] && [ `echo "$elem_time > $T0_time" | bc` -eq 1 ] && [ `echo "$elem_time < $T12_time + 0.04"  | bc` -eq 1 ]
    then
        in_flag=1
        category=`expr $iter % 15`
        segID=`expr $iter / 15 + 1`
        segID=${segIDArray[$segID]}
        if [ $category -eq 0 ]
        then
            echo "(T1) select tracks                        - OmafDashAccess :    ${total_T1toT13[$iter]}" >> "$summary_file_name".txt
            detectTimeAverage=`awk 'BEGIN{printf "%.9f\n", '$detectTimeAverage+$elem_time-$T0_time'}'`
	        detectInChanged=$(($detectInChanged+1))
        elif [ $category -eq 1 ]
        then
            echo "(T2) pose changed                         - OmafDashAccess :    ${total_T1toT13[$iter]}" >> "$summary_file_name".txt
        elif [ $category -eq 2 ]
        then
            echo "(T3) start to download segment $segID          - OmafDashAccess :    ${total_T1toT13[$iter]}" >> "$summary_file_name".txt
        elif [ $category -eq 3 ]
        then
            echo "(T4) start to parse segment $segID             - OmafDashAccess :    ${total_T1toT13[$iter]}" >> "$summary_file_name".txt
        elif [ $category -eq 4 ]
        then
            echo "(T5) end to parse segment $segID               - OmafDashAccess :    ${total_T1toT13[$iter]}" >> "$summary_file_name".txt
        elif [ $category -eq 5 ]
        then
            if [ "$packing_method" == "later binding" ]
            then
                echo "(T6) start to stitch frame at pts ${ptsArray[$segID]}      - OmafDashAccess :    ${total_T1toT13[$iter]}" >> "$summary_file_name".txt
            fi
        elif [ $category -eq 6 ]
        then
            if [ "$packing_method" == "later binding" ]
            then
                echo "(T7) end to stitch frame at pts ${ptsArray[$segID]}, and video number is ${videoNumArray[$segID]} - OmafDashAccess :    ${total_T1toT13[$iter]}" >> "$summary_file_name".txt
            fi
        elif [ $category -eq 7 ]
        then
            echo "(T8) get packet at pts ${ptsArray[$segID]}                         - player :    ${total_T1toT13[$iter]}" >> "$summary_file_name".txt
        elif [[ $category -gt 7 ]] && [[ $category -lt 13 ]]
        then
            if [ $max_video_num -gt $(($category-8)) ]
            then
            echo "(T9) push frame to decoded fifo at pts ${ptsArray[$segID]}, video id is $(($category-8))   - player :    ${total_T1toT13[$iter]}" >> "$summary_file_name".txt
            timeMS=`cat mthq_trace.txt | grep "T10_decode_time_cost" | grep "pts_field = ${ptsArray[$segID]}," |  grep -m 1 "video_id_field = $(($category-8))" | awk -F " " '{print $19}' | awk -F "," '{print $1}'`
            width=`cat mthq_trace.txt | grep "T10_decode_time_cost" | grep "pts_field = ${ptsArray[$segID]}," |  grep -m 1 "video_id_field = $(($category-8))" | awk -F " " '{print $22}'`
            height=`cat mthq_trace.txt | grep "T10_decode_time_cost" | grep "pts_field = ${ptsArray[$segID]}," |  grep -m 1 "video_id_field = $(($category-8))" | awk -F " " '{print $25}'`
            echo "(T10) decode one frame of $width x $height, time cost : $timeMS ms   - player" >> "$summary_file_name".txt
            fi
        elif [ $category -eq 13 ]
        then
            echo "(T11) update frame at pts ${ptsArray[$segID]}                      - player :    ${total_T1toT13[$iter]}" >> "$summary_file_name".txt
        elif [ $category -eq 14 ]
        then
            echo "(T13) render frame at pts ${ptsArray[$segID]}                      - player :    ${total_T1toT13[$iter]}" >> "$summary_file_name".txt
        fi
    fi
done
# # 5.echo the last T9' information if occurs in last segment
if [ `echo "$T12_time + 0.4 > $elem_time" | bc` -eq 1 ]
then
    echo "(T12) blit all high quality tiles in FOV at pts ${T12ptsArrary[$cur_T0]} - player :    ${T12Array[$cur_T0]}" >> "$summary_file_name".txt
    # echo T9n-T1 time interval
    timeCost=`awk 'BEGIN{printf "%.9f\n", '$T12_time-$T0_time'}'`
    echo "Motion to high quality time cost is                                    $timeCost" >> "$summary_file_name".txt
fi

echo "--------------------------------[Average Time Cost]--------------------------------" >> "$summary_file_name".txt
echo "Motion changes $(($motion_change_times-1)) times, in which $changedTimes of them occurs low quality in FOV" >> "$summary_file_name".txt
echo "Motion from low quality to high quality average time cost  :  $changedAVGTime s" >> "$summary_file_name".txt
totalAVGTime=`awk 'BEGIN{printf "%.9f\n", '$changedAVGTime\*$changedTimes/$(($motion_change_times-1))'}'`
echo "Total motion to high quality average time cost             :  $totalAVGTime s" >> "$summary_file_name".txt
echo "--------------------------------[Average Time Cost in sub modules]--------------------------------" >> "$summary_file_name".txt
if [ $detectInChanged -eq 0 ]
then
    detectTimeAverage=0
else
    detectTimeAverage=`awk 'BEGIN{printf "%.9f\n", '$detectTimeAverage/$detectInChanged'}'`
fi
echo "Detect pose change time cost                               :  $detectTimeAverage s" >> "$summary_file_name".txt
echo "Select tracks time cost                                    :  ${timeAverageArray[0]} s" >> "$summary_file_name".txt
echo "Change pose time cost                                      :  ${timeAverageArray[1]} s" >> "$summary_file_name".txt
echo "Download segment time cost                                 :  ${timeAverageArray[2]} s" >> "$summary_file_name".txt
echo "Parse segment time cost                                    :  ${timeAverageArray[3]} s" >> "$summary_file_name".txt
# echo "Wait for player getting first packet in segment time cost  :  ${timeAverageArray[4]} s" >> "$summary_file_name".txt
if [ "$packing_method" == "later binding" ]
then
echo "Stitch one frame time cost                                 :  ${timeAverageArray[5]} s" >> "$summary_file_name".txt
fi
echo "Get packet time cost                                       :  ${timeAverageArray[6]} s" >> "$summary_file_name".txt
echo "Decode frame and fifo wait time cost                       :  ${timeAverageArray[7]} s" >> "$summary_file_name".txt
echo "Update frame time cost                                     :  ${timeAverageArray[8]} s" >> "$summary_file_name".txt
echo "Render frame time cost                                     :  ${timeAverageArray[9]} s" >> "$summary_file_name".txt
fi