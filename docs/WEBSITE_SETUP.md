# ClankerOS Website Setup

This document explains the documentation website setup for ClankerOS.

## Overview

The ClankerOS documentation website is a static site that provides:

1. **Session Summaries** - High-level overviews of development sessions
2. **Detailed Chatlogs** - Full conversation logs from Claude Code sessions
3. **API Documentation** - Doxygen-generated code documentation

## Architecture

### Static Site Generation

The website is generated from source files using Python scripts:

- **`scripts/generate_website.py`** - Converts JSONL chatlog files to HTML
- **`scripts/convert_markdown.py`** - Converts Markdown session summaries to HTML
- **Doxygen** - Generates API documentation from code comments

### Directory Structure

```
clankeros/
├── docs/
│   ├── outputs/           # JSONL chatlog files (source)
│   └── sessions/          # Markdown session summaries (source)
├── website/
│   ├── index.html         # Main homepage (static)
│   ├── sessions.html      # Sessions index (generated)
│   ├── chatlogs.html      # Chatlogs index (generated)
│   ├── css/style.css      # Styles (static)
│   ├── sessions/          # Session HTML pages (generated)
│   ├── chatlogs/          # Chatlog HTML pages (generated)
│   └── api/               # Doxygen output (generated)
└── .github/workflows/
    └── docs.yml           # GitHub Actions deployment
```

## Building Locally

```bash
# Build everything
make website

# Serve locally on http://localhost:8000
make serve-website

# Build individual components
python3 scripts/generate_website.py   # Chatlogs
python3 scripts/convert_markdown.py   # Sessions
doxygen Doxyfile                      # API docs
```

## GitHub Pages Deployment

The website automatically deploys to GitHub Pages on every push to `main`.

### Setup Steps

1. **Enable GitHub Pages** in repository settings:
   - Go to Settings → Pages
   - Source: GitHub Actions

2. **Workflow runs automatically** on push to main:
   - Installs Python, Doxygen, Graphviz
   - Runs website generation scripts
   - Runs Doxygen
   - Deploys to GitHub Pages

3. **Website will be available at**:
   - `https://yourusername.github.io/clankeros/`

### Workflow File

The deployment is configured in [.github/workflows/docs.yml](../.github/workflows/docs.yml):

- Triggers on push to `main`
- Builds website using Python scripts
- Generates Doxygen documentation
- Deploys to GitHub Pages using official actions

## Content Management

### Adding Session Summaries

1. Create a new Markdown file in `docs/sessions/`:
   ```bash
   docs/sessions/session-03.md
   ```

2. Write summary in Markdown format (see existing files for examples)

3. Regenerate website:
   ```bash
   make website
   ```

4. Commit and push - GitHub Actions will deploy automatically

### Adding Chatlogs

1. Copy JSONL chatlog files to `docs/outputs/`:
   ```bash
   cp ~/.claude/projects/clankeros/SESSION_ID.jsonl docs/outputs/
   ```

2. Regenerate website:
   ```bash
   make website
   ```

3. The chatlog will automatically appear in the index

### Updating API Documentation

Doxygen extracts documentation from code comments. To update:

1. Add/update comments in source files:
   ```c
   /**
    * @brief Initialize the kernel
    * @param multiboot_info Pointer to multiboot information
    */
   void kernel_main(multiboot_info_t *multiboot_info);
   ```

2. Regenerate docs:
   ```bash
   make docs
   ```

## Customization

### Styling

Edit `website/css/style.css` to customize appearance.

Colors are defined in CSS variables:
```css
:root {
    --primary-color: #2c3e50;
    --secondary-color: #3498db;
    --text-color: #333;
    --bg-color: #f5f5f5;
}
```

### Homepage

Edit `website/index.html` directly to update the homepage content.

### Doxygen Configuration

Edit `Doxyfile` to customize API documentation:
- Project name/version
- Input directories
- Output settings
- Graph generation

## Features

### Session Summaries
- Converted from Markdown to HTML
- Preserves formatting (headers, lists, code blocks)
- Linked from sessions index page

### Chatlogs
- Parsed from JSONL format
- Color-coded by role (user/assistant/system/summary)
- Timestamps included
- Scrollable, readable format

### API Documentation
- Generated from source code
- Includes call graphs (requires Graphviz)
- Searchable
- Cross-referenced

## Privacy Considerations

The chatlog files contain your full conversation history, including:
- Working directory paths (contains username)
- Timestamps
- Full conversation content

Before publishing, consider:
1. Reviewing chatlog content for sensitive information
2. Sanitizing paths if needed (username appears in file paths)
3. Using the website generation scripts to create a curated view

## Maintenance

### Regenerating After Changes

```bash
# After adding new session summaries
python3 scripts/convert_markdown.py

# After adding new chatlogs
python3 scripts/generate_website.py

# After updating code documentation
doxygen Doxyfile

# All at once
make website
```

### Cleaning Generated Files

```bash
# Clean website output (preserves static files)
rm -rf website/chatlogs/*.html website/sessions/*.html website/api/

# Then regenerate
make website
```

## Troubleshooting

### Doxygen not found
```bash
# Ubuntu/Debian
sudo apt-get install doxygen graphviz

# macOS
brew install doxygen graphviz
```

### Python scripts fail
- Ensure Python 3.x is installed
- Scripts have no external dependencies
- Check that source files exist in `docs/outputs/` and `docs/sessions/`

### GitHub Pages not updating
- Check Actions tab for workflow failures
- Verify GitHub Pages is enabled in repository settings
- Check that workflow has write permissions

## Future Enhancements

Potential improvements:
- Search functionality across chatlogs
- Filtering/sorting of sessions
- Syntax highlighting for code blocks in chatlogs
- Statistics dashboard (commits, lines of code, sessions)
- Timeline view of development progress
