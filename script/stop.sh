#!/system/bin/sh

#==========================================================================
#   Copyright (C) 2019 wang. All rights reserved.
#   
#   Authors:    wanglongfei(395592722@qq.com)
#   Date:       2019/07/25 14:35:11
#   Description: 结束服务器运行
#
#==========================================================================

ps -ef | grep app | awk '{print $2}' | xargs kill -9
