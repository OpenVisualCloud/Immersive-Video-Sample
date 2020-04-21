#!/bin/bash

babeltrace2 ust/ > packing_trace.txt

curr_time=$(date "+%Y%m%d%H%M%S")

summary_file_name=$1

summary_file_name="$summary_file_name"_"$curr_time"

all_segs_size=0

intermediate_result1=`cat packing_trace.txt | grep "frame_resolution_field" | awk -F " " '{print $13}'`

width1=$(echo $intermediate_result1 | awk -F " " '{print $1}')
width1=$(echo $width1 | awk -F "\"" '{print $2}')

width2=$(echo $intermediate_result1 | awk -F " " '{print $2}')
width2=$(echo $width2 | awk -F "\"" '{print $2}')

intermediate_result2=`cat packing_trace.txt | grep "frame_resolution_field" | awk -F " " '{print $15}'`

height1=$(echo $intermediate_result2 | awk -F " " '{print $1}')
height1=$(echo $height1 | awk -F "\"" '{print $1}')

height2=$(echo $intermediate_result2 | awk -F " " '{print $2}')
height2=$(echo $height2 | awk -F "\"" '{print $1}')

resolution1=$(($width1*$height1))

resolution2=$(($width2*$height2))

intermediate_result3=`cat packing_trace.txt | grep "frame_resolution_field" | awk -F " " '{print $18}'`

tile_rows1=$(echo $intermediate_result3 | awk -F " " '{print $1}')
tile_rows1=$(echo $tile_rows1 | awk -F "\"" '{print $2}')

tile_rows2=$(echo $intermediate_result3 | awk -F " " '{print $2}')
tile_rows2=$(echo $tile_rows2 | awk -F "\"" '{print $2}')

intermediate_result4=`cat packing_trace.txt | grep "frame_resolution_field" | awk -F " " '{print $20}'`

tile_cols1=$(echo $intermediate_result4 | awk -F " " '{print $1}')
tile_cols1=$(echo $tile_cols1 | awk -F "\"" '{print $1}')

tile_cols2=$(echo $intermediate_result4 | awk -F " " '{print $2}')
tile_cols2=$(echo $tile_cols2 | awk -F "\"" '{print $1}')

echo "-------------------------Tiles Selection Basing On Viewport-------------------------" > "$summary_file_name".txt

if [ $resolution1 -gt $resolution2 ]
then
  echo "Bitstream 1:" >> "$summary_file_name".txt
  echo "  Resolution : $width1 x $height1" >> "$summary_file_name".txt
  echo "  Tiles split: $tile_rows1 x $tile_cols1" >> "$summary_file_name".txt
  high_res_width=$width1
  high_res_height=$height1
  high_tile_rows=$tile_rows1
  high_tile_cols=$tile_cols1
  low_res_width=$width2
  low_res_height=$height2
  echo "Bitstream 2:" >> "$summary_file_name".txt
  echo "  Resolution : $width2 x $height2" >> "$summary_file_name".txt
  echo "  Tiles split: $tile_rows2 x $tile_cols2" >> "$summary_file_name".txt
else
  echo "Bitstream 1:" >> "$summary_file_name".txt
  echo "  Resolution : $width2 x $height2" >> "$summary_file_name".txt
  echo "  Tiles split: $tile_rows2 x $tile_cols2" >> "$summary_file_name".txt
  high_res_width=$width2
  high_res_height=$height2
  high_tile_rows=$tile_rows2
  high_tile_cols=$tile_cols2
  low_res_width=$width1
  low_res_height=$height1
  echo "Bitstream 2:" >> "$summary_file_name".txt
  echo "  Resolution : $width1 x $height1" >> "$summary_file_name".txt
  echo "  Tiles split: $tile_rows1 x $tile_cols1" >> "$summary_file_name".txt
fi

viewport_set_width=`cat packing_trace.txt | grep "viewport_set_width" | awk -F " " '{print $13}'`
viewport_set_width=$(echo $viewport_set_width | awk -F "," '{print $1}')

viewport_set_height=`cat packing_trace.txt | grep "viewport_set_height" | awk -F " " '{print $16}'`
viewport_set_height=$(echo $viewport_set_height | awk -F "," '{print $1}')

viewport_set_pitch=`cat packing_trace.txt | grep "viewport_set_pitch" | awk -F " " '{print $19}'`
viewport_set_pitch=$(echo $viewport_set_pitch | awk -F "," '{print $1}')

viewport_set_yaw=`cat packing_trace.txt | grep "viewport_set_yaw" | awk -F " " '{print $22}'`
viewport_set_yaw=$(echo $viewport_set_yaw | awk -F "," '{print $1}')

horizontal_fov_angle=`cat packing_trace.txt | grep "horizontal_fov_angle" | awk -F " " '{print $25}'`
horizontal_fov_angle=$(echo $horizontal_fov_angle | awk -F "," '{print $1}')

vertical_fov_angle=`cat packing_trace.txt | grep "vertical_fov_angle" | awk -F " " '{print $28}'`
vertical_fov_angle=$(echo $vertical_fov_angle | awk -F "," '{print $1}')

projection_type=`cat packing_trace.txt | grep "projection_type_field" | awk -F " " '{print $31}'`
projection_type=$(echo $projection_type | awk -F "\"" '{print $2}')

echo "Viewport Info:" >> "$summary_file_name".txt
echo "  width                : $viewport_set_width" >> "$summary_file_name".txt
echo "  height               : $viewport_set_height" >> "$summary_file_name".txt
echo "  pitch                : $viewport_set_pitch" >> "$summary_file_name".txt
echo "  yaw                  : $viewport_set_yaw" >> "$summary_file_name".txt
echo "  FOV horizontal angle : $horizontal_fov_angle" >> "$summary_file_name".txt
echo "  FOV vertical angle   : $vertical_fov_angle" >> "$summary_file_name".txt
echo "  projection type      : $projection_type" >> "$summary_file_name".txt

calculated_net_width=`cat packing_trace.txt | grep "net_width_field" | awk -F " " '{print $13}'`
calculated_net_width=$(echo $calculated_net_width | awk -F "," '{print $1}')

calculated_net_height=`cat packing_trace.txt | grep "net_height_field" | awk -F " " '{print $16}'`
calculated_net_height=$(echo $calculated_net_height | awk -F "," '{print $1}')

calculated_tilealigned_width=`cat packing_trace.txt | grep "tiled_width_field" | awk -F " " '{print $19}'`
calculated_tilealigned_width=$(echo $calculated_tilealigned_width | awk -F "," '{print $1}')

calculated_tilealigned_height=`cat packing_trace.txt | grep "tiled_height_field" | awk -F " " '{print $22}'`
calculated_tilealigned_height=$(echo $calculated_tilealigned_height | awk -F "," '{print $1}')

selected_tile_rows=`cat packing_trace.txt | grep "tile_rows_field" | awk -F " " '{print $25}'`
selected_tile_rows=$(echo $selected_tile_rows | awk -F "," '{print $1}')

selected_tile_cols=`cat packing_trace.txt | grep "tile_cols_field" | awk -F " " '{print $28}'`
selected_tile_cols=$(echo $selected_tile_cols | awk -F "," '{print $1}')

selection_accurancy=`awk 'BEGIN{printf "%.2f%%\n", ('$(($calculated_net_width*$calculated_net_height))'/'$(($calculated_tilealigned_width*$calculated_tilealigned_height))')*100}'`

echo "Calculated viewport area : $calculated_net_width x $calculated_net_height" >> "$summary_file_name".txt
echo "Selected tiles layout    : $selected_tile_cols x $selected_tile_rows" >> "$summary_file_name".txt
echo "Selected tiles area      : $calculated_tilealigned_width x $calculated_tilealigned_height" >> "$summary_file_name".txt
echo "Calculated viewport area / Selected tiles area : $selection_accurancy" >> "$summary_file_name".txt

echo -e "\n" >> "$summary_file_name".txt
echo "-------------------------Server Side Bandwidth--------------------------------------" >> "$summary_file_name".txt
bitrate1=`cat packing_trace.txt | grep "video_bitrate_field" | awk -F " " '{print $31}'`
bitrate1=$(echo $bitrate1 | awk -F "," '{print $1}')
bitrate1=`awk 'BEGIN{printf "%d\n", '$(($bitrate1))' / 1000}'`

bitrate2=`cat packing_trace.txt | grep "video_bitrate_field" | awk -F " " '{print $34}'`
bitrate2=$(echo $bitrate2 | awk -F "," '{print $1}')
bitrate2=`awk 'BEGIN{printf "%d\n", '$(($bitrate2))' / 1000}'`

frame_rate=`cat packing_trace.txt | grep "frame_rate_field" | awk -F " " '{print $19}'`
frame_rate=$(echo $frame_rate | awk -F "," '{print $1}')

dash_mode=`cat packing_trace.txt | grep "dash_mode_field" | awk -F " " '{print $13}'`
dash_mode=$(echo $dash_mode | awk -F "\"" '{print $2}')

segment_dur=`cat packing_trace.txt | grep "segment_duration_field" | awk -F " " '{print $16}'`
segment_dur=$(echo $segment_dur | awk -F "," '{print $1}')

total_frames_num=`cat packing_trace.txt | grep "frames_per_video_field" | awk -F " " '{print $38}'`
total_frames_num=$(echo $total_frames_num | awk -F "," '{print $1}')

echo "Bitstream 1 encoding configuration:" >> "$summary_file_name".txt
echo "  Bitrate  : $bitrate1 Kbps" >> "$summary_file_name".txt
echo "  Framerate: $frame_rate" >> "$summary_file_name".txt
echo "Bitstream 2 encoding configuration:" >> "$summary_file_name".txt
echo "  Bitrate  : $bitrate2 Kbps" >> "$summary_file_name".txt
echo "  Framerate: $frame_rate" >> "$summary_file_name".txt
echo "DASH mode       : $dash_mode" >> "$summary_file_name".txt
echo "Segment duration: $segment_dur second" >> "$summary_file_name".txt
echo "Total frames num: $total_frames_num" >> "$summary_file_name".txt
echo "Bandwidth collection results:" >> "$summary_file_name".txt

all_frames_size=0

calculate_encoded_bitrate() {
width=$1
height=$2
curr_frame_rate=$3
frames_num=$4

num=`cat packing_trace.txt | grep "frame_resolution_field = \"$width x $height\"," |wc -l`

temp=`cat packing_trace.txt | grep "frame_resolution_field = \"$width x $height\"," | awk -F " " '{print $26}'`

total_frames_size=$(echo $temp | awk -F " " '{print $1}')

for i in $(seq 2 $num)
do
  one_frame_size=$(echo $temp | awk -F " " '{print $'$i'}')
  total_frames_size=$(($(($total_frames_size))+$(($one_frame_size))))
done

all_frames_size=$(($(($all_frames_size))+$(($total_frames_size))))
average_encoded_bitrate=`expr $total_frames_size \* 8 \* $curr_frame_rate / $frames_num / 1000`
echo "Bitstream $width x $height: average_encoded_bitrate = $average_encoded_bitrate Kbps" >> "$summary_file_name".txt
}

calculate_encoded_bitrate $high_res_width $high_res_height $frame_rate $total_frames_num
calculate_encoded_bitrate $low_res_width $low_res_height $frame_rate $total_frames_num


all_tile_segs_size=0

calculate_tile_average_bitrate() {
track_index=$1
frames_num=$2
curr_frame_rate=$3

num=`cat packing_trace.txt | grep "track_index_field = $track_index," |wc -l`
num=$(($num-1))

total_segs_size=`cat packing_trace.txt | grep "track_index_field = $track_index, track_type_field = \"init_track\"" | awk -F " " '{print $25}'`

temp=`cat packing_trace.txt | grep "track_index_field = $track_index, track_type_field = \"tile_track\"" | awk -F " " '{print $27}'`

for i in $(seq 1 $num)
do
  one_seg_size=$(echo $temp | awk -F " " '{print $'$i'}')
  total_segs_size=$(($(($total_segs_size))+$(($one_seg_size))))
done

average_bitrate=`expr $total_segs_size \* 8 \* $curr_frame_rate / $frames_num / 1000`

tile_width=`cat packing_trace.txt | grep "track_index_field = $track_index, track_type_field = \"tile_track\"" | awk -F " " '{print $19}'`
tile_width=$(echo $tile_width | awk -F "\"" '{print $2}')

tile_height=`cat packing_trace.txt | grep "track_index_field = $track_index, track_type_field = \"tile_track\"" | awk -F " " '{print $21}'`
tile_height=$(echo $tile_height | awk -F "\"" '{print $1}')

echo "tile track: track_index = $track_index, tile_resolution = $tile_width x $tile_height, average_packed_bitrate = $average_bitrate Kbps" >> "$summary_file_name".txt

all_tile_segs_size=$(($all_tile_segs_size+$total_segs_size))
all_segs_size=$(($all_segs_size+$total_segs_size))
}

tile_tracks_num=$(($(($tile_rows1*$tile_cols1))+$(($tile_rows2*$tile_cols2))))
for i in $(seq 1 $tile_tracks_num)
do
  calculate_tile_average_bitrate $i $total_frames_num $frame_rate
done

all_extractor_segs_size=0
first_extractortrack_id=1000

calculate_extractor_average_bitrate() {
track_index=$1
frames_num=$2
curr_frame_rate=$3

num=`cat packing_trace.txt | grep "track_index_field = $track_index," |wc -l`

temp=`cat packing_trace.txt | grep "track_index_field = $track_index," | awk -F " " '{print $25}'`

total_segs_size=$(echo $temp | awk -F " " '{print $1}')

for i in $(seq 2 $num)
do
  one_seg_size=$(echo $temp | awk -F " " '{print $'$i'}')
  total_segs_size=$(($(($total_segs_size))+$(($one_seg_size))))
done

average_bitrate=`expr $total_segs_size \* 8 \* $curr_frame_rate / $frames_num / 1000`

echo "extractor track: track_index = $track_index, average_packed_bitrate = $average_bitrate Kbps" >> "$summary_file_name".txt

if [ $track_index -eq $first_extractortrack_id ]
then
  all_extractor_segs_size=$(($all_extractor_segs_size+$total_segs_size))
  all_segs_size=$(($all_segs_size+$total_segs_size))
fi
}
extractor_tracks_num=$(($high_tile_rows*$high_tile_cols))

extractor_tracks_num=$(($extractor_tracks_num+999))
for j in $(seq 1000 $extractor_tracks_num)
do
  calculate_extractor_average_bitrate $j $total_frames_num $frame_rate
done


tile_segs_proportion=`awk 'BEGIN{printf "%.2f%%\n", ('$(($all_tile_segs_size))'/'$(($all_segs_size))') * 100}'`
extractor_segs_proportion=`awk 'BEGIN{printf "%.2f%%\n", ('$(($all_extractor_segs_size))'/'$(($all_segs_size))') * 100}'`
total_packed_bitrate=`expr $all_segs_size \* 8 \* $frame_rate / $total_frames_num / 1000`
encoding_to_packing_ratio=`awk 'BEGIN{printf "%.2f%%\n", ('$(($all_frames_size))'/'$(($all_segs_size))') * 100}'`

echo "Summary:" >> "$summary_file_name".txt
echo "total_packed_bitrate                                = $total_packed_bitrate Kbps" >> "$summary_file_name".txt
echo "tile_tracks_to_total_packed_bitrate_proportion      = $tile_segs_proportion" >> "$summary_file_name".txt
echo "extractor_tracks_to_total_packed_bitrate_proportion = $extractor_segs_proportion" >> "$summary_file_name".txt
echo "encoded_bitrate_to_packed_bitrate_proportion        = $encoding_to_packing_ratio" >> "$summary_file_name".txt
