#cmake file for project association
#author: liuwenjun
#created:2014/02/20

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

## 添加项目

set(CMAKE_SHARED_LIBRARY_SUFFIX ".passoa")
SET(CMAKE_SHARED_LIBRARY_PREFIX "")
if(MSVC)
  add_library(sqlite SHARED
	${PROJECT_SOURCE_DIR}/src/sqlite.cpp
	${PROJECT_SOURCE_DIR}/src/sqlite3.c
	${PROJECT_SOURCE_DIR}/src/export.def
	${PROJECT_SOURCE_DIR}/src/plugin.h
	)
else()
endif()
