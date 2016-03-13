CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(homescreen-efl C)

SET(INSTALL_EXEC_PREFIX "${INSTALL_PREFIX}/bin")
SET(INSTALL_RESDIR "${INSTALL_PREFIX}/res")
SET(INSTALL_IMAGEDIR "${INSTALL_RESDIR}/images")
SET(EDJEDIR "${INSTALL_RESDIR}/edje")
SET(LOCALEDIR "${INSTALL_RESDIR}/locale")
SET(CMAKE_VERBOSE_MAKEFILE 0)

SET(PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}/..")
SET(PROJECT_RESOURCES_DIR "${PROJECT_ROOT_DIR}/res")
SET(PROJECT_EDC_DIR "${PROJECT_RESOURCES_DIR}/edje")
SET(PROJECT_IMAGES_DIR "${PROJECT_RESOURCES_DIR}/images")
SET(PROJECT_PO_DIR "${PROJECT_RESOURCES_DIR}/po")

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

FOREACH (flag ${MENU_PKGS_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -fvisibility=hidden -Wall -Winline -g -fno-builtin-malloc -fPIC")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

INCLUDE_DIRECTORIES(${PROJECT_ROOT_DIR}/inc)

ADD_EXECUTABLE(${PROJECT_NAME}
	${PROJECT_ROOT_DIR}/src/homescreen-efl.c
	${PROJECT_ROOT_DIR}/src/all_apps.c
	${PROJECT_ROOT_DIR}/src/app_mgr.c
	${PROJECT_ROOT_DIR}/src/app_item.c
	${PROJECT_ROOT_DIR}/src/data_model.c
	${PROJECT_ROOT_DIR}/src/tree.c

	${PROJECT_ROOT_DIR}/src/app_grid.c
	${PROJECT_ROOT_DIR}/src/layout.c
	${PROJECT_ROOT_DIR}/src/page_scroller.c
	${PROJECT_ROOT_DIR}/src/util.c
	${PROJECT_ROOT_DIR}/src/app_icon.c
	${PROJECT_ROOT_DIR}/src/key.c
	${PROJECT_ROOT_DIR}/src/option_menu.c
	${PROJECT_ROOT_DIR}/src/mouse.c
	${PROJECT_ROOT_DIR}/src/folder_panel.c
	${PROJECT_ROOT_DIR}/src/popup.c
	${PROJECT_ROOT_DIR}/src/dynamic_index.c
	${PROJECT_ROOT_DIR}/src/livebox_all_pages.c
	${PROJECT_ROOT_DIR}/src/db/db.c

	${PROJECT_ROOT_DIR}/src/livebox/livebox_panel.c
	${PROJECT_ROOT_DIR}/src/livebox/livebox_animator.c
	${PROJECT_ROOT_DIR}/src/livebox/grid_item_resize.c
	${PROJECT_ROOT_DIR}/src/livebox/livebox_utils.c
	${PROJECT_ROOT_DIR}/src/livebox/grid_reposition.c
	${PROJECT_ROOT_DIR}/src/livebox/livebox_widget.c
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} -lm)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${MENU_PKGS_LDFLAGS})

# Install
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_EXEC_PREFIX})

INSTALL(FILES ${PROJECT_ROOT_DIR}/tizen-manifest.xml DESTINATION ${SYS_PACKAGES_DIR} RENAME org.tizen.homescreen-efl.xml)
INSTALL(DIRECTORY DESTINATION ${PREFIX}/data)
INSTALL(FILES ${PROJECT_ROOT_DIR}/shared/res/homescreen-efl.png DESTINATION ${SYS_ICONS_DIR})

# End of a file
ADD_SUBDIRECTORY(res)
