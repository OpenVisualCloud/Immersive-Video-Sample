#!/bin/bash

babeltrace2 ust/ > downloaded_trace.txt

curr_time=$(date "+%Y%m%d%H%M%S")
summary_file_name=$1
summary_file_name="$summary_file_name"_"$curr_time"

all_segs_size=0

dash_mode=`cat downloaded_trace.txt | grep "dash_mode_field" | awk -F " " '{print $13}'`
dash_mode=$(echo $dash_mode | awk -F "\"" '{print $2}')

segment_dur=`cat downloaded_trace.txt | grep "segment_duration_field" | awk -F " " '{print $16}'`
segment_dur=$(echo $segment_dur | awk -F "," '{print $1}')

frame_rate=`cat downloaded_trace.txt | grep "frame_rate_field" | awk -F " " '{print $19}'`
frame_rate=$(echo $frame_rate | awk -F "," '{print $1}')

total_frames_num=`cat downloaded_trace.txt | grep "frames_per_video_field" | awk -F " " '{print $35}'`
total_frames_num=$(echo $total_frames_num | awk -F "," '{print $1}')

presentation_dur=`awk 'BEGIN{printf "%.2f\n", '$(($total_frames_num))'/'$(($frame_rate))'}'`

init_segs_size=0
init_segs_num=`cat downloaded_trace.txt | grep "track_type_field = \"init_track\"" |wc -l`

temp=`cat downloaded_trace.txt | grep "track_type_field = \"init_track\"" | awk -F " " '{print $25}'`

for i in $(seq 1 $init_segs_num)
do
  one_init_seg_size=$(echo $temp | awk -F " " '{print $'$i'}')
  init_segs_size=$(($(($init_segs_size))+$(($one_init_seg_size))))
done

extractor_track_index=`cat downloaded_trace.txt | grep "extractor_index_field" | awk -F " " '{print $13}'`
extractor_track_index=$(echo $extractor_track_index | awk -F "," '{print $1}')

extractor_track_refs_num=`cat downloaded_trace.txt | grep "dependent_tracks_num_field" | awk -F " " '{print $16}'`
extractor_track_refs_num=$(echo $extractor_track_refs_num | awk -F "," '{print $1}')

echo "-------------------------Client Side Bandwidth--------------------------------------" >> "$summary_file_name".txt
echo "DASH mode                              : $dash_mode" >> "$summary_file_name".txt
echo "Segment duration                       : $segment_dur second" >> "$summary_file_name".txt
echo "Total presentation duration            : $presentation_dur seconds" >> "$summary_file_name".txt
echo "Media stream frame rate                : $frame_rate" >> "$summary_file_name".txt
echo "Total frames number                    : $total_frames_num" >> "$summary_file_name".txt
echo "Total initial segments number          : $init_segs_num" >> "$summary_file_name".txt
echo "Total initial segmnets size            : $init_segs_size Bytes" >> "$summary_file_name".txt
echo "Selected extractor track               : $extractor_track_index" >> "$summary_file_name".txt
echo "Extractor track reference tracks number: $extractor_track_refs_num" >> "$summary_file_name".txt
echo "Bandwidth collection results:" >> "$summary_file_name".txt

download_num=`cat downloaded_trace.txt | grep "download_info" |wc -l`

calculate_downloading_bitrate() {
segment_index=$1
segment_num=$2

temp=`cat downloaded_trace.txt | grep "download_info" | awk -F " " '{print $1}'`
download_start_time=$(echo $temp | awk -F " " '{print $'$segment_index'}')

download_start_time=$(echo $download_start_time | awk -F "[" '{print $2}')
download_start_time=$(echo $download_start_time | awk -F ":" '{print $3}')

download_time_second=$(echo $download_start_time | awk -F "." '{print $1}')
ten_second=`expr $download_time_second / 10`
one_second=`expr $download_time_second % 10`

compare_value=0
if [ "$ten_second" -eq "$compare_value" ]
then
  download_time_second=$one_second
fi

download_time_nsecond=$(echo $download_start_time | awk -F "." '{print $2}')
download_time_nsecond=$(echo $download_time_nsecond | awk -F "]" '{print $1}')

result=`expr $download_time_nsecond / 100000000`
result1=`expr $download_time_nsecond % 100000000`

if [ "$result" -eq "$compare_value" ]
then
  download_time_nsecond=$result1
fi

download_time=$(($(($download_time_second*1000000000))+$(($download_time_nsecond))))



temp1=`cat downloaded_trace.txt | grep "segment_index_field = $segment_index" | awk -F " " '{print $1}'`
download_end_time=$(echo $temp1 | awk -F " " '{print $'$segment_num'}')

download_end_time=$(echo $download_end_time | awk -F "[" '{print $2}')
download_end_time=$(echo $download_end_time | awk -F ":" '{print $3}')
download_end_second=$(echo $download_end_time | awk -F "." '{print $1}')
end_ten_second=`expr $download_end_second / 10`
end_one_second=`expr $download_end_second % 10`

if [ "$end_ten_second" -eq "$compare_value" ]
then
  download_end_second=$end_one_second
fi

download_end_nsecond=$(echo $download_end_time | awk -F "." '{print $2}')
download_end_nsecond=$(echo $download_end_nsecond | awk -F "]" '{print $1}')
result2=`expr $download_end_nsecond / 100000000`
result3=`expr $download_end_nsecond % 100000000`
if [ "$result2" -eq "$compare_value" ]
then
  download_end_nsecond=$result3
fi

end_time=$(($(($download_end_second*1000000000))+$(($download_end_nsecond))))

temp2=`cat downloaded_trace.txt | grep "segment_index_field = $segment_index," | awk -F " " '{print $25}'`
track_num=`cat downloaded_trace.txt | grep "segment_index_field = $segment_index," |wc -l`

total_segs_size=$(echo $temp2 | awk -F " " '{print $1}')

for i in $(seq 2 $track_num)
do
  one_seg_size=$(echo $temp2 | awk -F " " '{print $'$i'}')
  total_segs_size=$(($(($total_segs_size))+$(($one_seg_size))))
done

all_segs_size=$(($all_segs_size+$total_segs_size))

download_interval=$(($(($end_time))-$(($download_time))))

average_bitrate=`expr $total_segs_size \* 8 \* 1000000000 / $download_interval / 1000`
current_bitrate=`expr $total_segs_size \* 8 / 1000`

if [ "$max_bitrate" -lt "$current_bitrate" ]
then
  max_bitrate=$current_bitrate
fi

if [ "$min_bitrate" -gt "$current_bitrate" ]
then
  min_bitrate=$current_bitrate
fi

download_interval_ms=`awk 'BEGIN{printf "%.2f\n", '$(($download_interval))' / 1000000}'`

echo "downloading_bitrate_for_${segment_index}_segment_group = $average_bitrate Kbps, downloading_interval = $download_interval_ms ms" >> "$summary_file_name".txt
}

max_bitrate=1
min_bitrate=100000000000
for i in $(seq 1 $download_num)
do
  calculate_downloading_bitrate $i $(($extractor_track_refs_num+1))
done

average_downloading_bitrate=`expr $all_segs_size \* 8 \* $frame_rate / $total_frames_num / 1000`
echo -e "\n" >> "$summary_file_name".txt
echo "Client side bandwidth summary:" >> "$summary_file_name".txt
echo "average_downloading_bitrate = $average_downloading_bitrate Kbps" >> "$summary_file_name".txt
echo "maximum_downloading_bitrate = $max_bitrate Kbps" >> "$summary_file_name".txt
echo "minimum_downloading_bitrate = $min_bitrate Kbps" >> "$summary_file_name".txt
echo "Done"
