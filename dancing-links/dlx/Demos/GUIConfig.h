WINDOW_TITLE("Dancing Links!")

RUN_TESTS_MENU_OPTION()
MENU_ORDER("MatchmakerGUI.cpp",
           "DisasterGUI.cpp")

TEST_ORDER("DisasterTags.cpp",
           "DisasterLinks.cpp",
           "PartnerLinksTests.cpp")

TEST_BARRIER("MatchmakerGUI.cpp", "PartnerLinksTests.cpp")
TEST_BARRIER("DisasterGUI.cpp",   "DisasterLinks.cpp")
