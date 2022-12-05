WINDOW_TITLE("Dancing Links!")

RUN_TESTS_MENU_OPTION()
MENU_ORDER("MatchmakerGUI.cpp",
           "DisasterGUI.cpp")
           
TEST_ORDER("Matchmaker.cpp",
           "PerfectLinks.cpp",
           "DisasterLinks.cpp",
           "DisasterPlanning.cpp",)

TEST_BARRIER("MatchmakerGUI.cpp", "Matchmaker.cpp")
TEST_BARRIER("DisasterGUI.cpp",   "DisasterPlanning.cpp")
