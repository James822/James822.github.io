import os
import shutil

SRC_DIR = "src/"
POSTS_DIR = SRC_DIR + "posts/"

STYLES_FILE = "docs/styles.css"
TITLE_FILE = SRC_DIR + "title.html"
PANEL_FILE = SRC_DIR + "panel.html"
MAIN_PAGE_FILE = SRC_DIR + "main_page.html"

if __name__ == "__main__":
    print("OS: " + os.name)
    print()
    print("Building site...")

    # checking that directories exist
    if not os.path.isdir(SRC_DIR):
        print("bad path")
        exit()
    if not os.path.isdir(POSTS_DIR):
        print("bad path")
        exit()

    # checking that files exist
    if not os.path.isfile(STYLES_FILE):
        print("bad file")
        exit()
    if not os.path.isfile(TITLE_FILE):
        print("bad file")
        exit()
    if not os.path.isfile(PANEL_FILE):
        print("bad file")
        exit()
    if not os.path.isfile(MAIN_PAGE_FILE):
        print("bad file")
        exit()

        
    title_file = open(TITLE_FILE, "r")
    panel_file = open(PANEL_FILE, "r")
    main_page_file = open(MAIN_PAGE_FILE, "r")

    title_contents = title_file.read()
    panel_contents = panel_file.read()
    main_page_contents = main_page_file.read()

    title_file.close()
    panel_file.close()
    main_page_file.close()
    
    
    # building site
    
    title_marker_index = main_page_contents.find("<!--$title.html-->") # special HTML comment to denote where the title_contents should be inserted in a page
    panel_marker_index = main_page_contents.find("<!--$panel.html-->") # special HTML comment to denote where the panel_contents should be inserted in a page
    if title_marker_index == -1:
        print("title_marker issue")
        exit()
    if panel_marker_index == -1:
        print("panel_marker issue")
        exit()
    if not (title_marker_index < panel_marker_index):
        print("title_marker is NOT before panel_marker")
        exit()

    built_main_page_contents = main_page_contents[:title_marker_index] + title_contents + main_page_contents[title_marker_index:panel_marker_index] + panel_contents + main_page_contents[panel_marker_index:]

    index_file = open("docs/index.html", "w")
    index_file.write(built_main_page_contents)
    index_file.close()

    for filename in os.scandir("src/posts"):
        if filename.is_file():
            post_file = open(filename.path, "r")
            post_contents = post_file.read()
            post_file.close()

            title_marker_index = post_contents.find("<!--$title.html-->") # special HTML comment to denote where the title_contents should be inserted in a page
            panel_marker_index = post_contents.find("<!--$panel.html-->") # special HTML comment to denote where the panel_contents should be inserted in a page
            if title_marker_index == -1:
                print("title_marker issue")
                exit()
            if panel_marker_index == -1:
                print("panel_marker issue")
                exit()
            if not (title_marker_index < panel_marker_index):
                print("title_marker is NOT before panel_marker")
                exit()

            built_post_contents = post_contents[:title_marker_index] + title_contents + post_contents[title_marker_index:panel_marker_index] + panel_contents + post_contents[panel_marker_index:]

            docs_post_file = open("docs/" + filename.name, "w")
            docs_post_file.write(built_post_contents)
            docs_post_file.close()
            
    



    


