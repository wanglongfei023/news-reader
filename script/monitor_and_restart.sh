#!/system/bin/sh

#==========================================================================
#   Copyright (C) 2019 wang. All rights reserved.
#   
#   Authors:    wanglongfei(395592722@qq.com)
#   Date:       2019/07/28 00:25:50
#   Description: 循环检测服务器是否允许并邮件报警
#
#==========================================================================

PATH=`pwd`/../src
cd $PATH
MAIL_RECIVER='395592722@qq.com'
MAIL_THEME='News-Reader Warning'
MAIL_CONTENT='The news-reader process has benn off-line just now.'

function send_mail()
{
	echo $MAIL_CONTENT | mail -s $MAIL_THEME $MAIL_RECIVER
}


while true
do
  pid=$( ps -ef | grep './app' | grep -v grep | awk '{print $2}')
  if [ -z $pid ]
    then
	if [ -f "./app" ]
	then
	    nohup ./app &
		send_mail
    fi
  fi
  sleep 120
done
