#!/bin/bash
addr2line=$(find ~ -name xtensa-lx106-elf-addr2line)
java -jar EspStackTraceDecoder.jar $addr2line $1 $2