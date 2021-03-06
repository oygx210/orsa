# ---------------------------------------------------------------------------- #
# PROJECT :         ORSA : Orbit Reconstruction, Simulation and Analysis
# AUTHOR :          Pasquale Tricarico <tricaric@psi.edu>
# CONTRIBUTORS :    Pasquale Tricarico <tricaric@psi.edu>
#                   Vincent Reverdy (ESA-SOCIS 2011) <vince.rev@gmail.com>
# ---------------------------------------------------------------------------- #
# ORSA - Orbit Reconstruction, Simulation and Analysis
# Copyright (C) 2002-2010 Pasquale Tricarico <tricaric@psi.edu>
# Portions copyright (C) by their respective authors.
#   
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#   
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#   
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
# ---------------------------------------------------------------------------- #



# ----------------------------------- NAMES ----------------------------------- #
GET_FILENAME_COMPONENT(ORSA_CURRENT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
SET(ORSA_CURRENT_LIBSTATIC ${ORSA_CURRENT_NAME}${LIBSTATIC_SUFFIX})
SET(ORSA_CURRENT_LIBSHARED ${ORSA_CURRENT_NAME}${LIBSHARED_SUFFIX})
# ---------------------------------------------------------------------------- #



# ---------------------------------- HEADERS --------------------------------- #
SET(ORSA_CURRENT_HEADERS
angle.h
bodygroup.h
body.h
box.h
cache.h
chebyshev.h
covariance.h
crash.h
datetime.h
debug.h
double.h
euler.h
integrator.h
integrator_leapfrog.h
integrator_radau.h
interaction.h
interval.h
legendre.h
massDistribution.h
matrix.h
multifit.h
multimin.h
orbit.h
paul.h
paulMoment.h
print.h
propulsion.h
quaternion.h
reference_frame.h
shape.h
slerp.h
spline.h
statistic.h
unit.h
util.h
vector.h
)
# ---------------------------------------------------------------------------- #



# ---------------------------------- SOURCES --------------------------------- #
SET(ORSA_CURRENT_SOURCES
angle.cpp
body.cpp
bodygroup.cpp
box.cpp
covariance.cpp
crash.cpp
debug.cpp
double.cpp
euler.cpp
integrator.cpp
integrator_leapfrog.cpp
integrator_radau.cpp
interaction.cpp
legendre.cpp
matrix.cpp
multifit.cpp
multimin.cpp
orbit.cpp
paul.cpp
paulMoment.cpp
propulsion.cpp
quaternion.cpp
reference_frame.cpp
shape.cpp
unit.cpp
util.cpp
vector.cpp
)
# ---------------------------------------------------------------------------- #



# ------------------------------ LINK LIBRARIES ------------------------------ #
SET(ORSA_CURRENT_LINK_INCLUDE
${OPENSCENEGRAPH_INCLUDE_DIRS}
${GMP_INCLUDE_DIRS}
${GSL_INCLUDE_DIRS}
)

SET(ORSA_CURRENT_LINK_LIBRARIES
${QT_LIBRARIES} 
${OPENSCENEGRAPH_LIBRARIES}
${GMP_LIBRARIES}
${GSL_LIBRARIES}
)
# ---------------------------------------------------------------------------- #



# ----------------------------------- BUILD ---------------------------------- #
QT4_WRAP_CPP(ORSA_CURRENT_MOC ${ORSA_CURRENT_HEADERS})

ADD_LIBRARY(${ORSA_CURRENT_LIBSTATIC} STATIC ${ORSA_CURRENT_SOURCES} ${ORSA_CURRENT_MOC})
SET_TARGET_PROPERTIES(${ORSA_CURRENT_LIBSTATIC} PROPERTIES CLEAN_DIRECT_OUTPUT 1 OUTPUT_NAME ${ORSA_CURRENT_NAME})
ADD_LIBRARY(${ORSA_CURRENT_LIBSHARED} SHARED ${ORSA_CURRENT_SOURCES} ${ORSA_CURRENT_MOC})
SET_TARGET_PROPERTIES(${ORSA_CURRENT_LIBSHARED} PROPERTIES CLEAN_DIRECT_OUTPUT 1 OUTPUT_NAME ${ORSA_CURRENT_NAME})

TARGET_LINK_LIBRARIES(${ORSA_CURRENT_LIBSTATIC} ${ORSA_CURRENT_LINK_LIBRARIES})
TARGET_LINK_LIBRARIES(${ORSA_CURRENT_LIBSHARED} ${ORSA_CURRENT_LINK_LIBRARIES})
# ---------------------------------------------------------------------------- #
