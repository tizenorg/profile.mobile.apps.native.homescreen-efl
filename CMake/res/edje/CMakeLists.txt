ADD_CUSTOM_TARGET(home.edj
		COMMAND edje_cc -id ${PROJECT_EDC_DIR}/../images ${EDJE_CFLAGS}
		${PROJECT_EDC_DIR}/home.edc home.edj
		DEPENDS ${PROJECT_EDC_DIR}/home.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} home.edj)
INSTALL(FILES home.edj DESTINATION ${EDJEDIR})

ADD_CUSTOM_TARGET(icon.edj
		COMMAND edje_cc -id ${PROJECT_EDC_DIR}/../images ${EDJE_CFLAGS}
		${PROJECT_EDC_DIR}/icon.edc icon.edj
		DEPENDS ${PROJECT_EDC_DIR}/icon.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} icon.edj)
INSTALL(FILES icon.edj DESTINATION ${EDJEDIR})

ADD_CUSTOM_TARGET(folder.edj
		COMMAND edje_cc -id ${PROJECT_EDC_DIR}/../images ${EDJE_CFLAGS}
		${PROJECT_EDC_DIR}/folder.edc folder.edj
		DEPENDS ${PROJECT_EDC_DIR}/folder.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} folder.edj)
INSTALL(FILES folder.edj DESTINATION ${EDJEDIR})

ADD_CUSTOM_TARGET(folder_entry.edj
		COMMAND edje_cc -id ${PROJECT_EDC_DIR}/../images ${EDJE_CFLAGS}
		${PROJECT_EDC_DIR}/folder_entry.edc folder_entry.edj
		DEPENDS ${PROJECT_EDC_DIR}/folder_entry.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} folder_entry.edj)
INSTALL(FILES folder_entry.edj DESTINATION ${EDJEDIR})

ADD_CUSTOM_TARGET(index_element.edj
		COMMAND edje_cc -id ${PROJECT_EDC_DIR}/../images ${EDJE_CFLAGS}
		${PROJECT_EDC_DIR}/index_element.edc index_element.edj
		DEPENDS ${PROJECT_EDC_DIR}/index_element.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} index_element.edj)
INSTALL(FILES index_element.edj DESTINATION ${EDJEDIR})

ADD_CUSTOM_TARGET(app_grid.edj
		COMMAND edje_cc -id ${PROJECT_EDC_DIR}/../images ${EDJE_CFLAGS}
		${PROJECT_EDC_DIR}/app_grid.edc app_grid.edj
		DEPENDS ${PROJECT_EDC_DIR}/app_grid.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} app_grid.edj)
INSTALL(FILES app_grid.edj DESTINATION ${EDJEDIR})

ADD_CUSTOM_TARGET(livebox_container.edj
		COMMAND edje_cc -id ${PROJECT_EDC_DIR}/../images ${EDJE_CFLAGS}
		${PROJECT_EDC_DIR}/livebox_container.edc livebox_container.edj
		DEPENDS ${PROJECT_EDC_DIR}/livebox_container.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} livebox_container.edj)
INSTALL(FILES livebox_container.edj DESTINATION ${EDJEDIR})

ADD_CUSTOM_TARGET(livebox_layout.edj
		COMMAND edje_cc -id ${PROJECT_EDC_DIR}/../images ${EDJE_CFLAGS}
		${PROJECT_EDC_DIR}/livebox_layout.edc livebox_layout.edj
		DEPENDS ${PROJECT_EDC_DIR}/livebox_layout.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} livebox_layout.edj)
INSTALL(FILES livebox_layout.edj DESTINATION ${EDJEDIR})

ADD_CUSTOM_TARGET(page_edit.edj
		COMMAND edje_cc -id ${PROJECT_EDC_DIR}/../images ${EDJE_CFLAGS}
		${PROJECT_EDC_DIR}/page_edit.edc page_edit.edj
		DEPENDS ${PROJECT_EDC_DIR}/page_edit.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} page_edit.edj)
INSTALL(FILES page_edit.edj DESTINATION ${EDJEDIR})
