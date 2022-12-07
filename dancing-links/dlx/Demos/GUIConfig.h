WINDOW_TITLE("Dancing Links!")

RUN_TESTS_MENU_OPTION()
MENU_ORDER("MatchmakerGUI.cpp",
           "DisasterGUI.cpp")

TEST_ORDER("Matchmaker.cpp",
           "PartnerLinks.cpp",
           "DisasterLinks.cpp",
           "DisasterPlanning.cpp",)

TEST_BARRIER("MatchmakerGUI.cpp", "PartnerLinks.cpp")
TEST_BARRIER("DisasterGUI.cpp",   "DisasterLinks.cpp")
