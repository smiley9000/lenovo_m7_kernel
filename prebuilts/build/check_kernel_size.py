#!/usr/bin/python
# 
# Copyright Statement:
# --------------------
# This software is protected by Copyright and the information contained
# herein is confidential. The software may not be copied and the information
# contained herein may not be used or disclosed except with the written
# permission of MediaTek Inc. (C) 2010
# 
# BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
# NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
# SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
# 
# BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
# LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
# 
# THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
# WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
# LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
# RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
# THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
# 

import os
import re
import sys
import getopt
import platform
import shlex
import binascii
import struct
import subprocess
import signal, threading, time
from subprocess import *

global kernel_layout_header_path
global project_config_path
global project_name
global platform_name
global kernel_out
global dtb_name
global config_pattern
global config_pattern_len
global dynamic_kernel_enable
global dynamic_kernel_state

kernel_layout_header_path = ""
project_config_path = ""
project_name = ""
platform_name = ""
kernel_out = ""
dtb_name = ""
config_pattern = "CONFIG_MTK_PLATFORM="
config_pattern_len = 0
project_config_pattern = "MTK_DYNAMIC_KERNEL_LOADING = "
project_config_pattern_len = 0
global_nm_path = "prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin/aarch64-linux-android-nm"
dynamic_kernel_enable = 0
dynamic_kernel_state = ""

def get_info():
    global kernel_layout_header_path
    global project_config_path
    global kernel_out
    global dtb_name
    global config_pattern
    global config_pattern_len
    global project_config_pattern
    global project_config_pattern_len
    global project_name
    global platform_name
    global dynamic_kernel_enable
    global dynamic_kernel_state

    kernel_out = sys.argv[1]
    dtb_name = sys.argv[3]
    project_name = re.sub("mediatek/", "", dtb_name);
    config_file_path = kernel_out + '/.config'

    # get platform name
    if (os.path.exists(config_file_path)):
        file = open(config_file_path, 'r')
        platform_pattern = re.compile(config_pattern)
        config_pattern_len = len(config_pattern)
        for line in file.readlines():
            if (platform_pattern.search(line)):
                platform_name = eval(line[config_pattern_len:])

    kernel_layout_header_path = 'vendor/mediatek/proprietary/bootable/bootloader/lk/platform/' +\
                                platform_name+ '/include/platform/memory_layout.h'
    project_config_path = "device/mediateksample/" + project_name + "/ProjectConfig.mk"

    if (os.path.exists(project_config_path) == 0):
        project_config_path = "device/mediatekprojects/" + project_name + "/ProjectConfig.mk"
        if (os.path.exists(project_config_path) == 0):
            print('[warning] Can\'t find ther path %s\n'%config_file_path)
            sys.exit(0)
    if (os.path.exists(project_config_path)):
        file = open(project_config_path, 'r')
        dynamic_kernel_pattern = re.compile(project_config_pattern)
        project_config_pattern_len = len(project_config_pattern)
        for line in file.readlines():
            if (dynamic_kernel_pattern.search(line)):
                dynamic_kernel_state = (line[project_config_pattern_len:])
                if dynamic_kernel_state == 'yes\n':
                    dynamic_kernel_enable = 1
                    break
                else:
                    dynamic_kernel_enable = 0

if __name__ == '__main__':
    end_str   = ""
    stext_stop_str = ""
    end_hex   = 0x0
    stext_stop_hex = 0x0
    ksize_pa       = 0x0
    stext_hex      = 0x0

    get_info()

    nm_output_file = kernel_out + '/nm_output.txt'
    nm_output = open(nm_output_file, 'w')
    end_status_file = kernel_out + '/end.txt'
    end_status = open(end_status_file, 'w')
    stext_file = kernel_out + '/stext.txt'
    stext_status = open(stext_file, 'w')

    command = shlex.split(global_nm_path + ' ' + kernel_out + '/vmlinux')
    p = subprocess.Popen(command, stdout=nm_output);
    nm_output.close()
    time.sleep(2)

    command1 = shlex.split('grep " _end" ' +  nm_output_file)
    q = subprocess.Popen(command1, stdout=end_status);
    end_status.close()
    time.sleep(1)

    command1 = shlex.split('grep "T _stext" ' +  nm_output_file)
    q = subprocess.Popen(command1, stdout=stext_status);
    stext_status.close()
    time.sleep(1)

    if (os.path.exists(end_status_file)):
        file = open(end_status_file, 'r')
        for info in file.readlines():
            if (info.split()[2] == "_end"):
                end_str = str(info.split()[0])
                end_hex = int(end_str,16)
            else:
                print('[warning] Can\'t get symbol _end\n')
                sys.exit(0)
        if (end_hex <= 0):
            print('[warning] _end(0x%x) should be larger than 0\n'%end_hex)
            sys.exit(0)
    else:
        print('[warning] File: ' + end_status_file + ' doesn\'t exist')
        sys.exit(0)

    if (os.path.exists(stext_file)):
        file = open(stext_file, 'r')
        for info in file.readlines():
            if (info.split()[2] == "_stext"):
                stext_str = str(info.split()[0])
                stext_hex = int(stext_str,16)
            else:
                print('[warning] Can\'t get symbol "_stext"\n')

        if (stext_hex <= 0):
            print('[warning] end(0x%x) should be larger than 0\n'%stext_hex)
            sys.exit(0)
    else:
        print('[warning] File: ' + stext_file + ' doesn\'t exist')
        sys.exit(0)

    #get kernel size
    if (dynamic_kernel_enable == 1):
        print("dynamic_kernel_enable %d\n"%dynamic_kernel_enable)
        if (os.path.exists(kernel_layout_header_path)):
            file = open(kernel_layout_header_path, 'r')
            for line in file.readlines():
                ksize_pattern = re.compile(r'LK_DYNAMIC_KERNEL_64_MAX_SIZE')
                if (ksize_pattern.search(line)):
                    _ksize_pa = str(line.split()[2]).strip('(').strip(')')
                    ksize_pa = int(_ksize_pa,16)
                    break
    else:
        if(os.path.exists(kernel_layout_header_path) and ksize_pa == 0x0):
            file = open(kernel_layout_header_path, 'r')
            for line in file.readlines():
                ksize_pattern = re.compile(r'LK_KERNEL_64_MAX_SIZE')
                if (ksize_pattern.search(line)):
                    _ksize_pa = str(line.split()[2]).strip('(').strip(')')
                    ksize_pa = int(_ksize_pa,16)
                    break
    if (ksize_pa <= 0):
        print('[warning] ksize_pa(0x%x) should be larger than 0\n'%ksize_pa)
        sys.exit(0)

    # Check end - stext in vmlinux should be smaller than LK_KERNEL_64_MAX_SIZE
    if ((end_hex - stext_hex) > ksize_pa):
        print('[error] kernel image size too large')
        raise BaseException('current kernel image(0x%x) > layout setting(0x%x)\n'%((end_hex-stext_hex), ksize_pa))
    else:
        print('[pass] check kernel image size is pass')
