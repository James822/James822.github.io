# This script static assembles the site and deploys it to the docs/ directory which github pages takes and then actually builds the site
# this script MUST exist at the root directory of the project/repository so that it can navigate the directory structure properly with relative paths (all paths are configured relative to the project root)

import os

SRC_DIR = "src/"
DOCS_DIR = "docs/"
POSTS_DIR = SRC_DIR + "posts/"
DOCS_POSTS_DIR = DOCS_DIR + "posts/"

TEMPLATE_FILE = SRC_DIR + "template.html"
# WELCOME_PAGE = SRC_DIR + "welcome_page.html"
CATEGORIES_PAGE = SRC_DIR + "categories_page.html"
PROJECT_PAGE = SRC_DIR + "project_page.html"
ABOUT_PAGE = SRC_DIR + "about_page.html"

dirs = []
dirs.append(SRC_DIR)
dirs.append(DOCS_DIR)
dirs.append(POSTS_DIR)
dirs.append(DOCS_POSTS_DIR)

files = []
files.append(TEMPLATE_FILE)
# files.append(WELCOME_PAGE)
files.append(CATEGORIES_PAGE)
files.append(PROJECT_PAGE)
files.append(ABOUT_PAGE)

if __name__ == "__main__":
    print("Building site...")

    # checking that directories exist
    for dir in dirs:
        if not os.path.isdir(dir):
            print("ERROR: directory: \"" + str(dir) + "\" not found, check to make sure paths and directory structure is configured properly")
            exit()

    # checking that files exist
    for f in files:
        if not os.path.isfile(f):
            print("ERROR: file: \"" + str(f) + "\" not found, check to make sure paths and directory structure is configured properly")
            exit()

            
    # assembling template base file to use for every page
    marker = "@POST_CONTENT"
    template_file = open(TEMPLATE_FILE, "r")
    template_file_contents = template_file.read()
    index = template_file_contents.find(marker)
    if index == -1:
        print("ERROR: " + marker + " marker not found in the template file, cannot build site")
        exit()
    tf_first = template_file_contents[0:index]
    tf_second = template_file_contents[index+len(marker):]
    # print(tf_first)
    # print(tf_second)

    
    # building categories page (index.html)
    cat_page_file = open(CATEGORIES_PAGE, "r")
    cat_page_file_contents = cat_page_file.read()

    index_page_file = open(DOCS_DIR + "index.html", "w")
    index_page_file.write(tf_first + cat_page_file_contents + tf_second)

    
    # # building categories page
    # cat_page_file = open(CATEGORIES_PAGE, "r")
    # cat_page_file_contents = cat_page_file.read()
    
    # docs_cat_page_file = open(DOCS_DIR + "categories_page.html", "w")
    # docs_cat_page_file.write(tf_first + cat_page_file_contents + tf_second)

    
    # building project page
    project_page_file = open(PROJECT_PAGE, "r")
    project_page_file_contents = project_page_file.read()
    
    docs_project_page_file = open(DOCS_DIR + "project_page.html", "w")
    docs_project_page_file.write(tf_first + project_page_file_contents + tf_second)

    
    # building about page
    about_page_file = open(ABOUT_PAGE, "r")
    about_page_file_contents = about_page_file.read()
    
    docs_about_page_file = open(DOCS_DIR + "about_page.html", "w")
    docs_about_page_file.write(tf_first + about_page_file_contents + tf_second)
    

    # building all post pages
    for filename in os.scandir(POSTS_DIR):
        # print(filename.name)
        if not filename.name.startswith('.'):
            # building this post page
            post_page_file = open(POSTS_DIR + filename.name, "r")
            post_page_file_contents = post_page_file.read()

            docs_post_page_file = open(DOCS_POSTS_DIR + filename.name, "w")
            docs_post_page_file.write(tf_first + post_page_file_contents + tf_second)
            

    # @POST_CONTENT
