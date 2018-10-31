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
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/d2a2928e/c4.o


# C Compiler Flags
CFLAGS=-fPIC

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../lib/libinternal.${CND_CONF}-${CND_PLATFORM}.${CND_DLIB_EXT}

../lib/libinternal.${CND_CONF}-${CND_PLATFORM}.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ../lib
	${LINK.c} -o ../lib/libinternal.${CND_CONF}-${CND_PLATFORM}.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/_ext/d2a2928e/c4.o: ../core/c4.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/d2a2928e
	${RM} "$@.d"
	$(COMPILE.c) -g -DCND_PLATFORM=\"${CND_PLATFORM}\" -DCONF_${CND_CONF}=1 -DPROJECTNAME=\"${PROJECTNAME}\" -I../include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d2a2928e/c4.o ../core/c4.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ../lib/libinternal.${CND_CONF}-${CND_PLATFORM}.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
