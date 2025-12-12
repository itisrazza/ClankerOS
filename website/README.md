# ClankerOS Website

This directory contains the static website for ClankerOS documentation.

## Building the Website

The website is automatically built by GitHub Actions on every push to main. To build locally:

```bash
# Build everything (generates website + API docs)
make website

# Or build step by step:
python3 scripts/generate_website.py   # Convert JSONL chatlogs to HTML
python3 scripts/convert_markdown.py   # Convert session summaries to HTML
doxygen Doxyfile                      # Generate API documentation
```

## Local Development

To preview the website locally:

```bash
make serve-website
# Opens http://localhost:8000
```

## Structure

```
website/
├── index.html           # Homepage
├── sessions.html        # Session summaries index (generated)
├── chatlogs.html        # Chatlogs index (generated)
├── css/
│   └── style.css       # Main stylesheet
├── sessions/           # Generated session summary pages (from docs/sessions/*.md)
├── chatlogs/           # Generated chatlog pages (from docs/outputs/*.jsonl)
└── api/                # Doxygen-generated API documentation
```

## Content Sources

- **Chatlogs**: Generated from `docs/outputs/*.jsonl` files
- **Session Summaries**: Generated from `docs/sessions/*.md` files
- **API Docs**: Generated from source code comments in `kernel/` and `libraries/`

## Deployment

The website is deployed to GitHub Pages automatically via `.github/workflows/docs.yml`:

1. On push to main, GitHub Actions builds the website
2. Runs both Python scripts to generate HTML from JSONL and Markdown
3. Runs Doxygen to generate API documentation
4. Deploys everything to GitHub Pages

## Adding New Content

### New Session Summary

1. Create `docs/sessions/session-XX.md` with markdown content
2. Run `make website` to regenerate
3. Session will appear on the Sessions page

### New Chatlog

1. Copy JSONL file to `docs/outputs/`
2. Run `make website` to regenerate
3. Chatlog will appear on the Chatlogs page

The website generation scripts automatically detect new files.
