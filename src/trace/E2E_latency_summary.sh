#!/bin/bash -e

curr_time=$(date "+%Y%m%d%H%M%S")

summary_file_name="$summary_file_name"_"$curr_time"

############################################################
# List of field in lttng trace header file and categories
############################################################
f_list="da_ssi_info \
        pre_de_info \
        post_de_info \
        pre_op_info \
        post_op_info \
        pre_da_info \
        post_da_info \
        pre_rd_info \
        post_rd_info"
c_list="encode \
        server \
        client \
        overall"
f_list=(${f_list[@]})
c_list=(${c_list[@]})

############################################################
# Get specific event line in target frame.
# Globals:
#   tracefile_name
# Arguments:
#   $1: pts of target frame
#   $2: event to collect in this frame
# Returns:
#   The value of event [return as echo]
# Case:
#   get_target_frame_field 12 "Thrift_receive_info"
############################################################

get_target_frame_line(){
    line=`cat ${tracefile_name} | \
              grep "pts_field = \b$1\b" | \
              grep "\b$2\b"`
    echo -e "${line}"
}

############################################################
# Convert local time to total time.
# Globals:
#   None
# Arguments:
#   $1: local time
# Returns:
#   The value of total time [return as echo]
# Case:
#   get_target_frame_time "Thrift_receive_info"
############################################################

calc_time()
{
    form=$1
    if [[ -n "${form}" ]] ; then
        local_time=(${form//./ })
        second=`date -d ${local_time[0]} +%s`
        nano_second=`awk 'BEGIN{printf "%.9f\n", '${local_time[1]}/1000000000'}'`
        total_time=`awk 'BEGIN{printf "%.9f\n", '${second}+${nano_second}'}'`
        echo $total_time
    else
        echo ""
    fi
}

############################################################
# Get timestamp of certrain field result in target frame.
# Globals:
#   None
# Arguments:
#   $1: lines of target index
#   $2: field to collect in this event
#   $3: target timestamp attribution
# Returns:
#   timestamp of target frame [return as echo]
# Case:
#   get_target_timestamp "${lines}" "pre_de_info" first
############################################################

get_target_timestamp(){
    lines=$1
    attri=$2
    field=`echo "$2" | awk -F '.' '{print $1}'`
    tail_flag=`echo "$2" | awk -F '.' '{print $2}'`
    TS=`echo "${lines}" | grep "${field}"`
    if [ -n "${attri}" ] ; then
        TS=`echo "${TS}" | grep "${attri}"`
    fi

    TS=`echo "${TS}" | awk -F '[' '{print $2}' | awk -F ']' '{print $1}'`
    TS=(${TS[@]})
    if [ "${tail_flag}" == "tail" ] ; then
        echo "`calc_time ${TS[-1]}`"
    else
        echo "`calc_time ${TS[0]}`"
    fi
}

############################################################
# Get index of first segment
# Globals:
#   tracefile_name
# Arguments:
# Returns:
#   The value of event [return as echo]
# Case:
#   get_start_segment_index
############################################################

get_start_segment_index(){
    line=`cat ${tracefile_name} | \
              grep "da_ssi_info" | \
              head -1 | \
              awk -F ' ' '{print $13}'`
    echo -e "${line}"
}

############################################################
# Get the gap of frame index between server and client
# Globals:
# Arguments:
#   fps
# Returns:
#   The value of gap [return as echo]
# Case:
#   get_frame_gap
############################################################

get_frame_gap(){
    fps=$1
    let ssi=`get_start_segment_index`-1
    sfi=`awk 'BEGIN{printf "%d\n", '${ssi}*${fps}'}'`
    echo -e "${sfi}"
}

############################################################
# Show results of frames in whole pipeline
# Globals:
#   field_list
#   tracefile_name
# Arguments:
#   $1: pts of target frame on client
# Returns:
#   results of target frame [return as echo]
# Case:
#   single_frame_results_rough 56
############################################################

frames_results_rough(){
    tracefile_name=$1
    fps=$2
    start_num=$3
    end_num=$4
    scale=$5
    high_port=9090
    low_port=9089
    splitline="-------------------------------------------------"
    gap=`get_frame_gap ${fps}`
    valid_count=0
    echo "gap: ${gap}"
    echo ${scale}${splitline}${splitline}

    # encode results
    if [ "${scale}" == "encode" ] ; then
        printf "        %10s \t%10s \t%10s \n" \
               "frame:" "adjcent-pre_de" "adjcent-post_de"
        for index in $(seq ${start_num} ${end_num})
        do
            let target=${index}+${gap}
            let next=${target}+1
            target_lines=`cat ${tracefile_name} | grep "idx_field = \b${target}\b"`
            next_lines=`cat ${tracefile_name} | grep "idx_field = \b${next}\b"`
            t_pre=`get_target_timestamp "${target_lines}" "${f_list[1]}"`
            n_pre=`get_target_timestamp "${next_lines}" "${f_list[1]}"`
            t_post=`get_target_timestamp "${target_lines}" "${f_list[2]}"`
            n_post=`get_target_timestamp "${next_lines}" "${f_list[2]}"`
            if [[ -n "${t_pre}" ]] ; then
                D_pre=`awk 'BEGIN{printf "%.9f\n", '${n_pre}-${t_pre}'}'`
                D_post=`awk 'BEGIN{printf "%.9f\n", '${n_post}-${t_post}'}'`
                printf "        %10d \t%10f \t%10f \n" \
                        ${index} ${D_pre} ${D_post}
                let valid_count=${valid_count}+1
                TD_pre=`awk 'BEGIN{printf "%.9f\n", '${TD_pre}+${D_pre}'}'`
                TD_post=`awk 'BEGIN{printf "%.9f\n", '${TD_post}+${D_post}'}'`
            else
                printf "        %10d \tincomplete \n" ${index}
            fi
            done
        AD_pre=`awk 'BEGIN{printf "%.9f\n", '${TD_pre}/${valid_count}'}'`
        AD_post=`awk 'BEGIN{printf "%.9f\n", '${TD_post}/${valid_count}'}'`
        printf "        %15s \t%10f \n \t%15s \t%10f \n" \
               "ave-pre_de" ${AD_pre} "ave-post_de" ${AD_post}
    # server results
    elif [ "${scale}" == "server" ] ; then
        printf "        %10s \t%10s \t%10s \t%10s \t%10s  |  \t%10s \t%10s \n" \
               "frame:" "T4-T1" "T4-T3" "T3-T2" "T2-T1" "H_T2-T1" "L_T2-T1"
        for index in $(seq ${start_num} ${end_num})
        do
            let target=${index}+${gap}
            server_lines=`cat ${tracefile_name} | grep "idx_field = \b${target}\b"`
            T1=`get_target_timestamp "${server_lines}" "${f_list[1]}"`
            T1H=`get_target_timestamp "${server_lines}" "${f_list[1]}" ${high_port}`
            T1L=`get_target_timestamp "${server_lines}" "${f_list[1]}" ${low_port}`
            T2=`get_target_timestamp "${server_lines}" "${f_list[2]}"`
            T2H=`get_target_timestamp "${server_lines}" "${f_list[2]}" ${high_port}`
            T2L=`get_target_timestamp "${server_lines}" "${f_list[2]}" ${low_port}`
            T3=`get_target_timestamp "${server_lines}" "${f_list[3]}"`
            T4=`get_target_timestamp "${server_lines}" "${f_list[4]}"`
            if [[ -n "${T1}" ]] ; then
                D_T4T1=`awk 'BEGIN{printf "%.9f\n", '${T4}-${T1}'}'`
                D_T4T3=`awk 'BEGIN{printf "%.9f\n", '${T4}-${T3}'}'`
                D_T3T2=`awk 'BEGIN{printf "%.9f\n", '${T3}-${T2}'}'`
                D_T2T1=`awk 'BEGIN{printf "%.9f\n", '${T2}-${T1}'}'`
                D_HT2T1=`awk 'BEGIN{printf "%.9f\n", '${T2H}-${T1H}'}'`
                D_LT2T1=`awk 'BEGIN{printf "%.9f\n", '${T2L}-${T1L}'}'`
                printf "        %10d \t%10f \t%10f \t%10f \t%10f  |  \t%10f \t%10f \n" \
                        ${index} ${D_T4T1} ${D_T4T3} ${D_T3T2} ${D_T2T1} ${D_HT2T1} ${D_LT2T1}
                let valid_count=${valid_count}+1
                TD_T4T1=`awk 'BEGIN{printf "%.9f\n", '${TD_T4T1}+${D_T4T1}'}'`
                TD_T4T3=`awk 'BEGIN{printf "%.9f\n", '${TD_T4T3}+${D_T4T3}'}'`
                TD_T3T2=`awk 'BEGIN{printf "%.9f\n", '${TD_T3T2}+${D_T3T2}'}'`
                TD_T2T1=`awk 'BEGIN{printf "%.9f\n", '${TD_T2T1}+${D_T2T1}'}'`
                TD_HT2T1=`awk 'BEGIN{printf "%.9f\n", '${TD_HT2T1}+${D_HT2T1}'}'`
                TD_LT2T1=`awk 'BEGIN{printf "%.9f\n", '${TD_LT2T1}+${D_LT2T1}'}'`
            else
                printf "        %10d \tincomplete \n" ${index}
            fi
        done
        AD_T4T1=`awk 'BEGIN{printf "%.9f\n", '${TD_T4T1}/${valid_count}'}'`
        AD_T4T3=`awk 'BEGIN{printf "%.9f\n", '${TD_T4T3}/${valid_count}'}'`
        AD_T3T2=`awk 'BEGIN{printf "%.9f\n", '${TD_T3T2}/${valid_count}'}'`
        AD_T2T1=`awk 'BEGIN{printf "%.9f\n", '${TD_T2T1}/${valid_count}'}'`
        AD_HT2T1=`awk 'BEGIN{printf "%.9f\n", '${TD_HT2T1}/${valid_count}'}'`
        AD_LT2T1=`awk 'BEGIN{printf "%.9f\n", '${TD_LT2T1}/${valid_count}'}'`
        printf "        %15s \t%10f \n \t%15s \t%10f \n" \
               "ave-D_T4T1" ${AD_T4T1} "ave-D_T4T3" ${AD_T4T3}
        printf "        %15s \t%10f \n \t%15s \t%10f \n" \
               "ave-D_T3T2" ${AD_T3T2} "ave-D_T2T1" ${AD_T2T1}
        printf "        %15s \t%10f \n \t%15s \t%10f \n" \
               "ave-D_HT2T1" ${AD_HT2T1} "ave-D_LT2T1" ${AD_LT2T1}
    # client results
    elif [ "${scale}" == "client" ] ; then
        printf "        %10s \t%10s \t%10s \t%10s \t%10s \n" \
               "frame:" "T8-T4" "T8-T7" "T7-T6" "T6-T4"
        for index in $(seq ${start_num} ${end_num})
        do
            let target=${index}+${gap}
            server_lines=`cat ${tracefile_name} | grep "idx_field = \b${target}\b"`
            client_lines=`cat ${tracefile_name} | grep "idx_field = \b${index}\b"`
            T4=`get_target_timestamp "${server_lines}" "${f_list[4]}"`
            # T5=`get_target_timestamp "${client_lines}" "${f_list[5]}"`
            T6=`get_target_timestamp "${client_lines}" "${f_list[6]}"`
            T7=`get_target_timestamp "${client_lines}" "${f_list[7]}"`
            T8=`get_target_timestamp "${client_lines}" "${f_list[8]}"`
            if [[ -n "${T4}" ]] && [[ -n "${T8}" ]] ; then
                D_T8T4=`awk 'BEGIN{printf "%.9f\n", '${T8}-${T4}'}'`
                D_T8T7=`awk 'BEGIN{printf "%.9f\n", '${T8}-${T7}'}'`
                D_T7T6=`awk 'BEGIN{printf "%.9f\n", '${T7}-${T6}'}'`
                D_T6T4=`awk 'BEGIN{printf "%.9f\n", '${T6}-${T4}'}'`
                printf "        %10d \t%10f \t%10f \t%10f \t%10f \n" \
                        ${index} ${D_T8T4} ${D_T8T7} ${D_T7T6} ${D_T6T4}
                let valid_count=${valid_count}+1
                TD_T8T4=`awk 'BEGIN{printf "%.9f\n", '${TD_T8T4}+${D_T8T4}'}'`
                TD_T8T7=`awk 'BEGIN{printf "%.9f\n", '${TD_T8T7}+${D_T8T7}'}'`
                TD_T7T6=`awk 'BEGIN{printf "%.9f\n", '${TD_T7T6}+${D_T7T6}'}'`
                TD_T6T4=`awk 'BEGIN{printf "%.9f\n", '${TD_T6T4}+${D_T6T4}'}'`
            else
                printf "        %10d \tincomplete \n" ${index}
            fi
        done
        AD_T8T4=`awk 'BEGIN{printf "%.9f\n", '${TD_T8T4}/${valid_count}'}'`
        AD_T8T7=`awk 'BEGIN{printf "%.9f\n", '${TD_T8T7}/${valid_count}'}'`
        AD_T7T6=`awk 'BEGIN{printf "%.9f\n", '${TD_T7T6}/${valid_count}'}'`
        AD_T6T4=`awk 'BEGIN{printf "%.9f\n", '${TD_T6T4}/${valid_count}'}'`
        printf "        %15s \t%10f \n \t%15s \t%10f \n" \
               "ave-D_T8T4" ${AD_T8T4} "ave-D_T8T7" ${AD_T8T7}
        printf "        %15s \t%10f \n \t%15s \t%10f \n" \
               "ave-D_T7T6" ${AD_T7T6} "ave-D_T6T4" ${AD_T6T4}
    # overall results
    else
        unset TD_T8T4 TD_T4T1
        printf "        %10s \t%10s \t%10s \t%10s \n" \
               "frame:" "T8-T1" "T8-T4" "T4-T1"
        for index in $(seq ${start_num} ${end_num})
        do
            let target=${index}+${gap}
            server_lines=`cat ${tracefile_name} | grep "idx_field = \b${target}\b"`
            client_lines=`cat ${tracefile_name} | grep "idx_field = \b${index}\b"`
            T1=`get_target_timestamp "${server_lines}" "${f_list[1]}"`
            T4=`get_target_timestamp "${server_lines}" "${f_list[4]}"`
            T8=`get_target_timestamp "${client_lines}" "${f_list[8]}"`
            if [[ -n "${T1}" ]] && [[ -n "${T8}" ]] ; then
                D_T8T1=`awk 'BEGIN{printf "%.9f\n", '${T8}-${T1}'}'`
                D_T8T4=`awk 'BEGIN{printf "%.9f\n", '${T8}-${T4}'}'`
                D_T4T1=`awk 'BEGIN{printf "%.9f\n", '${T4}-${T1}'}'`
                printf "        %10d \t%10f \t%10f \t%10f \n" \
                        ${index} ${D_T8T1} ${D_T8T4} ${D_T4T1}
                let valid_count=${valid_count}+1
                TD_T8T1=`awk 'BEGIN{printf "%.9f\n", '${TD_T8T1}+${D_T8T1}'}'`
                TD_T8T4=`awk 'BEGIN{printf "%.9f\n", '${TD_T8T4}+${D_T8T4}'}'`
                TD_T4T1=`awk 'BEGIN{printf "%.9f\n", '${TD_T4T1}+${D_T4T1}'}'`
            else
                printf "        %10d \tincomplete \n" ${index}
            fi
        done
        AD_T8T1=`awk 'BEGIN{printf "%.9f\n", '${TD_T8T1}/${valid_count}'}'`
        AD_T8T4=`awk 'BEGIN{printf "%.9f\n", '${TD_T8T4}/${valid_count}'}'`
        AD_T4T1=`awk 'BEGIN{printf "%.9f\n", '${TD_T4T1}/${valid_count}'}'`
        printf "        %15s \t%10f \n \t%15s \t%10f \n \t%15s \t%10f \n" \
               "ave-D_T8T1" ${AD_T8T1} "ave-D_T8T4" ${AD_T8T4} "ave-D_T4T1" ${AD_T4T1}
    fi
    echo "valid_count: ${valid_count}"
}

# frames_results_rough ./4K_sample.log 30 1 10 encode
# frames_results_rough ./4K_sample.log 30 1 10 server
# frames_results_rough ./4K_sample.log 30 1 10 client
# frames_results_rough ./4K_sample.log 30 1 10 overall

trace_files=`ls ./copy/*.log`
output_path="./trace_output"
mkdir -p ${output_path}
for tracefile in ${trace_files}
do
    outputname=`basename ${tracefile} .log`
    outputfile="${output_path}/${outputname}_results.log"
    echo "------------------------"
    echo "${outputfile}"
    resolution=`echo ${outputname} | awk -F_ '{print $1}'`
    fps=`echo ${outputname} | awk -F_ '{print $2}'`

    for category in ${c_list[*]}
    do
        echo "${category} ${fps}"
        results=`frames_results_rough ${tracefile} ${fps} 1 10 ${category}`
        echo "${results}" >> ${outputfile}
    done
done

files=`ls ./${output_path}/*_results.log`
for file in ${files}
do
    echo "------------------------ ${file}"
    tail -n 4 ${file}
done
