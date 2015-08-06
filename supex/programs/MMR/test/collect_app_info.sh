#/bin/bash
#收集应用运行数据（应用名称、时间、连接数、CPU%、内存%）
#collect_app_inf.sh app_name(process_name) recore_file_name
#crontab运行
TIME_INFO=""
CPU_INFO=""
MEM_INFO=""
CONNECT_INFO=""

function get_time
{
	TIME_INFO=`date +%Y%m%d-%H%M%S`
}

function get_mem
{
	local APP_NAME=$1
	MEM_INFO=`ps -C $APP_NAME -o pmem|grep -v "%"`
}

function get_cpu
{
	local APP_NAME=$1
	CPU_INFO=`ps -C $APP_NAME -o pcpu|grep -v "%"`
}

function get_connect_count
{
	local APP_NAME=$1
	CONNECT_INFO=`netstat -anpt|grep $APP_NAME|grep "ESTABLISHED" -c`
}

function append_record
{
	local APP_NAME=$1
	local RECORD_FILE=$2
	if [ -z $APP_NAME ]; then
		echo "app_name is null"
	fi
	if [ -e $RECORD_FILE ]&&[ ! -f $RECORD_FILE ]; then
		echo "record_file is null"
	fi
	if [ ! -e $RECORD_FILE ]; then
		local dir_name=`dirname $RECORD_FILE`
		if [ ! -e $dir_name ]; then
			mkdir -p $dir_name
			if [ $? -ne 0 ]; then
				echo "mkdir $dir_name failed"
				exit 1
			fi
		fi
	fi
	get_time $APP_NAME
	get_cpu $APP_NAME
	get_mem $APP_NAME
	get_connect_count
	if [ ! -s $RECORD_FILE ]; then
		printf "%-20s %-20s %-20s %-20s %-20s\n" "APP_NAME" "TIME_INFO" "CONNECT_INFO" "CPU_INFO" "MEM_INFO" >>$RECORD_FILE
	fi
	printf "%-20s %-20s %-20s %-20s %-20s\n" "$APP_NAME" "$TIME_INFO" "$CONNECT_INFO" "$CPU_INFO" "$MEM_INFO" >>$RECORD_FILE
}

append_record $1 $2
