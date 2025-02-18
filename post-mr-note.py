#!/usr/bin/env python3

import sys, os, urllib3, argparse
# Silence the irritating insecure warnings.
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

# Add the location of python-gitlab to the path so we can import it
repo_top = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), '..'))
python_gitlab_dir = os.path.join(repo_top, "external/python-gitlab")
sys.path.append(python_gitlab_dir)
import gitlab


def getArgs():
    """
    Parse the command line arguments.
    Usage: python post-mr-note.py [PROJECT-TOKEN] [PROJECT] [MR-IID]
    Sample usage: python post-mr-note.py [PROJECT-TOKEN] documentation/latex-sample-chip-errata-repo 19
    """
    parser = argparse.ArgumentParser(
        description='Post note to existing MR.',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument( "authkey", type=str, help="Project or personal access token for authentication with GitLab. Create one at https://gitlab.com/profile/personal_access_tokens" )
    parser.add_argument( "project", type=str, help="Path to GitLab project in the form <namespace>/<project>")
    parser.add_argument( "mr_iid", type=str, help="Merge request number to post the message")
    parser.add_argument( "--url",  help="Gitlab URL.")
    return parser, parser.parse_args()


class PythonGitlabNotes():
    """
    Collect links to artifacts saved by other jobs in logs folder.
    Post the links as a note to an existing merge request.
    """
    def __init__(self, url, authkey, project, mr_iid):
        # Parse command line arguments
        self.url = url
        self.url_api = url + '/api/v4'
        self.authkey = authkey
        self.project_name = project
        self.mr_iid = mr_iid

        # Create python-gitlab server instance
        server = gitlab.Gitlab(self.url, myargs.authkey, api_version=4, ssl_verify=False)
        # Get an instance of the project and store it off
        self.project = server.projects.get(self.project_name)


    def collect_data(self):
        """
        Read document URLs from logs and organize them by language and chip series.
        """
        series_links_html = {}
        series_links_pdf = {}

        try:
            with open("logs/doc-url.txt", "r") as file:
                for line in file:
                    if line.startswith('[document preview]'):
                        tokens = line.split(']')
                        if len(tokens) < 3:
                            continue

                        desc_url = tokens[2].strip()
                        lang_chip_info = tokens[1].strip('[]')
                        lang_chip = lang_chip_info.split('_')

                        if len(lang_chip) == 2:
                            language = 'en'
                            chip_series = lang_chip[1]
                        elif len(lang_chip) == 3:
                            language = 'zh_CN'
                            chip_series = lang_chip[2]
                        else:
                            continue  # Skip malformed lines

                        # Construct new PDF URL
                        pdf_url = desc_url.replace(
                            "index.html",
                            f"esp-dev-kits-{language}-master-{chip_series}.pdf"
                        )

                        # Store links
                        if 'pdf' in pdf_url:
                            series_links_pdf.setdefault(chip_series, {})[language] = pdf_url
                        if 'html' in desc_url:
                            series_links_html.setdefault(chip_series, {})[language] = desc_url
        except FileNotFoundError:
            print("Error: logs/doc-url.txt not found.")
            sys.exit(1)

        self.series_links_html = series_links_html
        self.series_links_pdf = series_links_pdf

        # Debugging line
        print("HTML Links:", series_links_html)
        print("PDF Links:", series_links_pdf)

    def prepare_note(self):
        """
        Prepare a note with links to be posted in .md format.
        """
        note = "Documentation preview:\n\n"

        product_names = {
            'esp32': 'ESP32', 'esp32s2': 'ESP32-S2', 'esp32s3': 'ESP32-S3',
            'esp32c3': 'ESP32-C3', 'esp32c6': 'ESP32-C6', 'esp32h2': 'ESP32-H2',
            'esp32c2': 'ESP32-C2', 'esp32p4': 'ESP32-P4', 'esp32c5': 'ESP32-C5',
            'esp32c61': 'ESP32-C61', 'other': 'Other'
        }

        # Process both HTML and PDF links
        for chip_series, language_links_html in self.series_links_html.items():
            product_name = product_names.get(chip_series, "Unknown")
            note += f"- **{product_name}**\n"

            # Append HTML link if available
            if 'zh_CN' in language_links_html:
                note += f"\t * HTML: [esp-dev-kits 文档]({language_links_html['zh_CN']})/"
            else:
                note += f"esp-dev-kits 文档/"
            if 'en' in language_links_html:
                note += f"[esp-dev-kits Documentation]({language_links_html['en']}) \n"
            else:
                note += "esp-dev-kits Documentation"

            # Check if there are PDF links for the same chip series
            if chip_series in self.series_links_pdf:
                language_links_pdf = self.series_links_pdf[chip_series]
                if 'zh_CN' in language_links_pdf:
                    note += f"\t * PDF: [esp-dev-kits 文档]({language_links_pdf['zh_CN']})/"
                else:
                    note += f"esp-dev-kits 文档/"
                if 'en' in language_links_pdf:
                    note += f"[esp-dev-kits Documentation]({language_links_pdf['en']}) \n"
                else:
                    note += "esp-dev-kits Documentation"

            note += "\n"


        # Store the note in the instance variable
        self.note = note

        # Print the note for debugging
        print(note)

    def post_note(self):
        """
        Post the note to the merge request.
        """
        # Get the MR
        mr = self.project.mergerequests.get(self.mr_iid)
        # Post the note to the MR
        mr.notes.create({'body': self.note})

    def run(self):
        self.collect_data()
        self.prepare_note()
        self.post_note()


if __name__ == '__main__':
    myParser, myargs = getArgs()
    sys.exit(PythonGitlabNotes(url=myargs.url, authkey=myargs.authkey,
        project=myargs.project, mr_iid=myargs.mr_iid).run())
