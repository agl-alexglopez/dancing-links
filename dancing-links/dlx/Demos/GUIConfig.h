WINDOW_TITLE("Dancing Links!")

RUN_TESTS_MENU_OPTION()
MENU_ORDER("MatchmakerGUI.cpp",
           "DisasterGUI.cpp")

TEST_ORDER("PerformanceTests.cpp",
           "DisasterLinks.cpp",
           "DisasterPairs.cpp",
           "Matchmaker.cpp",
           "PartnerLinks.cpp",
           "DisasterPlanning.cpp",)

TEST_BARRIER("MatchmakerGUI.cpp", "PartnerLinks.cpp")
TEST_BARRIER("DisasterGUI.cpp",   "DisasterLinks.cpp")
