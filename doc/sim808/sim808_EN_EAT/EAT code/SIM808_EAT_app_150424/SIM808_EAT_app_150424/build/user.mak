##########################################################################
# File:             user.mak 
# Description:      User config file.
# Version:          0.1 
# Author:           Maobin
# DATE              NAME                        DESCRIPTION
# 2013-01-10        Maobin                      Initial Version 0.1
#########################################################################

#RVCT 3.1 Root Directory
DIR_ARM  = C:\Progra~1\ARM\RVCT

#生成二进制文件名
BIN = app

#Core 库所在目录
CORE_DIR = core

#源文件所在目录
SRC_DIR = demo

#编译输出文件目录
OUTPUT_BIN_DIR = output
OUTPUT_LIB_DIR = output/obj
OUTPUT_LOG_DIR = output/log

#模块名--模块名和该模块源代码所在目录名一样
#SRC_OPEN_MODE_NAME = adc audio flash fs gpio inputmethod interrupt key pwm spi system timer uart main wmmp i2c

#编译的模块名
SRC_OPEN_MODE_NAME = gps

#宏定义
# FEA_APP_EXAMPLE_OPT = -DAPP_EXAMPLE
#在源程序中使用方法
# #ifdef APP_EXAMPLE
# do something...;
# #endif
FEA_APP_EXAMPLE_OPT = -DAPP_EXAMPLE
FEA_APP_EXAMPLE2_OPT = -DAPP_EXAMPLE2
####################### WMMP begin ###########################
#FEA_APP_WMMP_OPT = -D__SIMCOM_WMMP__
#eat wmmp interface
#FEA_APP_WMMP_INTERFACE_OPT = -D__SIMCOM_EAT_WMMP__
#wmmp lib feature
#FEA_APP_WMMP30_OPT = -D__MODEM_WMMP30_SIMCOM__
#FEA_APP_WMMP_TEST_PLATFORM_OPT = -D__SIMCOM_WMMP_TEST_PLATFORM__
####################### WMMP end ############################
#FEA_APP_MULTI_APP_OPT = -D__SIMCOM_EAT_MULTI_APP__
 #First app or second app
#FEA_APP_MULTI_APP_INDEX_APP_OPT = -D__EAT_SECOND_APP__ #__EAT_FIRST_APP__

FEA_APP_CFLAGS = $(FEA_APP_EXAMPLE_OPT) \
				 $(FEA_APP_EXAMPLE2_OPT) \
				 $(FEA_APP_WMMP_OPT) \
				 $(FEA_APP_WMMP_INTERFACE_OPT) \
				 $(FEA_APP_WMMP30_OPT) \
				 $(FEA_APP_WMMP_TEST_PLATFORM_OPT) \
				 $(FEA_APP_MULTI_APP_OPT) \
				 $(FEA_APP_MULTI_APP_INDEX_APP_OPT)

