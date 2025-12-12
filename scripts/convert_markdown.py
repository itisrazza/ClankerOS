#!/usr/bin/env python3
"""
Convert markdown session summaries to HTML.
Uses python-markdown library for robust markdown parsing.
"""

import subprocess
from pathlib import Path
from html import escape
import markdown

# Project root
PROJECT_ROOT = Path(__file__).parent.parent
DOCS_DIR = PROJECT_ROOT / 'docs' / 'sessions'
WEBSITE_DIR = PROJECT_ROOT / 'website' / 'sessions'


def get_github_url():
    """Get GitHub repository URL from git remote."""
    try:
        result = subprocess.run(
            ['git', 'remote', 'get-url', 'origin'],
            cwd=PROJECT_ROOT,
            capture_output=True,
            text=True,
            check=True
        )
        url = result.stdout.strip()

        # Convert SSH URL to HTTPS
        if url.startswith('git@github.com:'):
            url = url.replace('git@github.com:', 'https://github.com/')
        if url.endswith('.git'):
            url = url[:-4]

        return url
    except:
        return 'https://github.com/yourusername/clankeros'


def convert_markdown_to_html(md_text):
    """Convert markdown to HTML using python-markdown library."""
    md = markdown.Markdown(extensions=['fenced_code', 'tables', 'nl2br', 'sane_lists'])
    return md.convert(md_text)


def generate_session_html(title, md_content, filename, github_url):
    """Generate HTML page for a session summary."""
    html_content = convert_markdown_to_html(md_content)
    source_url = f"{github_url}/blob/main/docs/sessions/{filename}"

    html = f'''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{escape(title)} - ClankerOS</title>
    <link rel="stylesheet" href="../css/style.css">
</head>
<body>
    <div class="container">
        <h1><a href="../index.html">ClankerOS</a></h1>
        <p class="meta">
            <a href="../sessions.html">← Back to sessions</a> |
            <a href="{escape(source_url)}">View source on GitHub</a>
        </p>

{html_content}
    </div>
</body>
</html>
'''

    return html


def main():
    """Convert all markdown files to HTML."""
    print("Converting session summaries to HTML...")

    # Get GitHub URL
    github_url = get_github_url()

    WEBSITE_DIR.mkdir(parents=True, exist_ok=True)

    for md_file in sorted(DOCS_DIR.glob('*.md')):
        print(f"  Converting {md_file.name}...")

        md_content = md_file.read_text()
        # Extract title from first line
        title = md_content.split('\n')[0].strip('# ')

        html = generate_session_html(title, md_content, md_file.name, github_url)

        output_file = WEBSITE_DIR / md_file.name.replace('.md', '.html')
        output_file.write_text(html)

        print(f"    → {output_file.name}")

    print(f"\n✓ Session summaries converted")


if __name__ == '__main__':
    main()
