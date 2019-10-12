#!/bin/python

# from SCons.Script import DefaultEnvironment
# env = DefaultEnvironment()

Import("env")

# # access to global build environment
# print(env)


def before_spiffs(source, target, env):
  env.Execute("./build-web-app.sh")

# env.AddPreAction("buildfs", before_spiffs)
env.AddPreAction(".pio/build/%s/spiffs.bin" % env['PIOENV'], before_spiffs)
# env.AddPreAction(".pio/build/pre_and_post_hooks/spiffs.bin", before_spiffs)

# print("XXXXX script %s" % env['PIOENV'])
