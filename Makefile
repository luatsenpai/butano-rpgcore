# Minimal Butano RPG framework extracted from Reversed Clock.
TARGET          := rc_framework
BUILD           := build
LIBBUTANO       := E:/work/gba/butano/butano
PYTHON          := python
SOURCES         := src
INCLUDES        := include
DATA            :=
GRAPHICS        := graphics
AUDIO           :=
AUDIOBACKEND    := null
AUDIOTOOL       :=
DMGAUDIO        :=
DMGAUDIOBACKEND := null
ROMTITLE        := RPGFRAMEWORK
ROMCODE         := RPGF
USERFLAGS       :=
USERCXXFLAGS    :=
USERASFLAGS     :=
USERLDFLAGS     :=
USERLIBDIRS     :=
USERLIBS        :=
DEFAULTLIBS     :=
STACKTRACE      :=
USERBUILD       :=
EXTTOOL         :=

ifndef LIBBUTANOABS
    export LIBBUTANOABS := $(realpath $(LIBBUTANO))
endif

include $(LIBBUTANOABS)/butano.mak
