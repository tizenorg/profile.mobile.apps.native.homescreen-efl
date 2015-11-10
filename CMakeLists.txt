CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(homescreen-efl C)

SET(MANIFESTDIR "/usr/share/packages")
SET(PACKAGE_NAME "org.tizen.${PROJECT_NAME}")
SET(PREFIX "/usr/apps/${PACKAGE_NAME}")
SET(EXEC_PREFIX "${PREFIX}/bin")
SET(DATADIR "${PREFIX}/data")
SET(RESDIR "${PREFIX}/res")
SET(IMAGEDIR "${RESDIR}/images")
SET(EDJEDIR "${RESDIR}/edje")
SET(LOCALEDIR "${RESDIR}/locale")
SET(VERSION 0.1.0)
SET(CMAKE_VERBOSE_MAKEFILE 0)

INCLUDE(FindPkgConfig)
pkg_check_modules(MENU_PKGS REQUIRED
	appsvc
	badge
	capi-appfw-application
	capi-system-info
	capi-appfw-package-manager
	capi-appfw-app-manager
	capi-system-system-settings
	dlog
	elementary
	shortcut
	syspopup-caller
	widget_viewer
	widget_service
	widget_viewer_evas
	capi-appfw-preference
	sqlite3
)

ADD_DEFINITIONS("-DENABLE_BGIMAGE")
ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")
ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE_NAME}\"")
ADD_DEFINITIONS("-DVERSION=\"${VERSION}\"")
ADD_DEFINITIONS("-DDATADIR=\"${DATADIR}\"")
ADD_DEFINITIONS("-DRESDIR=\"${RESDIR}\"")
ADD_DEFINITIONS("-DIMAGEDIR=\"${IMAGEDIR}\"")
ADD_DEFINITIONS("-DEDJEDIR=\"${EDJEDIR}\"")

FOREACH (flag ${MENU_PKGS_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -fvisibility=hidden -Wall -Winline -g -fno-builtin-malloc -fpie")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/inc)

ADD_EXECUTABLE(${PROJECT_NAME}
	src/homescreen-efl.c
	src/all_apps.c
	src/app_mgr.c
	src/app_item.c
	src/data_model.c
	src/tree.c

	src/app_grid.c
	src/layout.c
	src/page_scroller.c
	src/util.c
	src/app_icon.c
	src/key.c
	src/option_menu.c
	src/mouse.c
	src/folder_panel.c
	src/popup.c
	src/dynamic_index.c
	src/livebox_all_pages.c
	src/db/db.c

	src/livebox/livebox_panel.c
	src/livebox/livebox_animator.c
	src/livebox/grid_item_resize.c
	src/livebox/livebox_utils.c
	src/livebox/grid_reposition.c
	src/livebox/livebox_widget.c
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} -lm)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${MENU_PKGS_LDFLAGS})

# Install
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${EXEC_PREFIX})

ADD_SUBDIRECTORY(res)

CONFIGURE_FILE(${PACKAGE_NAME}.xml ${CMAKE_BINARY_DIR}/${PACKAGE_NAME}.xml)
INSTALL(FILES ${CMAKE_BINARY_DIR}/org.tizen.homescreen-efl.xml DESTINATION ${MANIFESTDIR})
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PACKAGE_NAME}.efl DESTINATION /etc/smack/accesses.d)
INSTALL(DIRECTORY DESTINATION ${PREFIX}/data)

# End of a file
