set(PIZZO_APP_NAME Pizzo)

set(PIZZO_CORE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/core/Graph.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/core/Loader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/core/Dijkstra.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/core/Matching.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/core/Euler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/core/CPP.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/core/Route.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/blackjack/Card.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/blackjack/Hand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/blackjack/BlackjackGame.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/blackjack/Advisor.cpp
)

set(PIZZO_CORE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/src/core/Graph.h
    ${CMAKE_CURRENT_LIST_DIR}/src/core/Loader.h
    ${CMAKE_CURRENT_LIST_DIR}/src/core/Dijkstra.h
    ${CMAKE_CURRENT_LIST_DIR}/src/core/Matching.h
    ${CMAKE_CURRENT_LIST_DIR}/src/core/Euler.h
    ${CMAKE_CURRENT_LIST_DIR}/src/core/CPP.h
    ${CMAKE_CURRENT_LIST_DIR}/src/core/Route.h
    ${CMAKE_CURRENT_LIST_DIR}/src/blackjack/Card.h
    ${CMAKE_CURRENT_LIST_DIR}/src/blackjack/Hand.h
    ${CMAKE_CURRENT_LIST_DIR}/src/blackjack/BlackjackGame.h
    ${CMAKE_CURRENT_LIST_DIR}/src/blackjack/Advisor.h
)

add_library(pizzo_core STATIC
    ${PIZZO_CORE_SOURCES}
    ${PIZZO_CORE_HEADERS}
)

target_include_directories(pizzo_core PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
target_compile_features(pizzo_core PUBLIC cxx_std_20)

set(PIZZO_APP_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
)

set(PIZZO_APP_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/src/Application.h
    ${CMAKE_CURRENT_LIST_DIR}/src/MainWindow.h
    ${CMAKE_CURRENT_LIST_DIR}/src/BlackjackWindow.h
    ${CMAKE_CURRENT_LIST_DIR}/src/GameState.h
    ${CMAKE_CURRENT_LIST_DIR}/src/ui/CommonTypes.h
    ${CMAKE_CURRENT_LIST_DIR}/src/ui/MainView.h
    ${CMAKE_CURRENT_LIST_DIR}/src/ui/BlackjackView.h
)

set(PIZZO_PLIST ${CMAKE_CURRENT_LIST_DIR}/res/appIcon/AppIcon.plist)

add_executable(${PIZZO_APP_NAME}
    ${PIZZO_APP_SOURCES}
    ${PIZZO_APP_HEADERS}
)

target_include_directories(${PIZZO_APP_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)
target_compile_features(${PIZZO_APP_NAME} PRIVATE cxx_std_20)

target_link_libraries(${PIZZO_APP_NAME}
    PRIVATE pizzo_core
    debug ${MU_LIB_DEBUG}
    debug ${NATGUI_LIB_DEBUG}
    optimized ${MU_LIB_RELEASE}
    optimized ${NATGUI_LIB_RELEASE}
)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/src PREFIX "src" FILES
    ${PIZZO_CORE_SOURCES}
    ${PIZZO_CORE_HEADERS}
    ${PIZZO_APP_SOURCES}
    ${PIZZO_APP_HEADERS}
)

setTargetPropertiesForGUIApp(${PIZZO_APP_NAME} ${PIZZO_PLIST})
setIDEPropertiesForGUIExecutable(${PIZZO_APP_NAME} ${CMAKE_CURRENT_LIST_DIR})
setPlatformDLLPath(${PIZZO_APP_NAME})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PIZZO_APP_NAME})
