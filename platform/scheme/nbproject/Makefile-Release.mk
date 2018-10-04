#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/scheme.o \
	${OBJECTDIR}/scheme_internal.o \
	${OBJECTDIR}/syscalls.o


# C Compiler Flags
CFLAGS=-Wno-format

# CC Compiler Flags
CCFLAGS=-Wno-format
CXXFLAGS=-Wno-format

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../../lib/libscheme.${CND_CONF}-${CND_PLATFORM}.${CND_DLIB_EXT}

../../lib/libscheme.${CND_CONF}-${CND_PLATFORM}.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ../../lib
	${LINK.cc} -o ../../lib/libscheme.${CND_CONF}-${CND_PLATFORM}.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/scheme.o: nbproject/Makefile-${CND_CONF}.mk scheme.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DCND_PLATFORM=\"${CND_PLATFORM}\" -DNDEBUG -DRELEASE -I../../include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/scheme.o scheme.c

${OBJECTDIR}/scheme_internal.o: nbproject/Makefile-${CND_CONF}.mk scheme_internal.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCND_PLATFORM=\"${CND_PLATFORM}\" -DCPP14 -DNDEBUG -DRELEASE -I../../include -std=c++14 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/scheme_internal.o scheme_internal.cpp

${OBJECTDIR}/syscalls.o: nbproject/Makefile-${CND_CONF}.mk syscalls.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DCND_PLATFORM=\"${CND_PLATFORM}\" -DNDEBUG -DRELEASE -I../../include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/syscalls.o syscalls.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ../../lib/libscheme.${CND_CONF}-${CND_PLATFORM}.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
