#!/usr/bin/env python3
"""
Generate static website for ClankerOS documentation.
Converts JSONL chatlogs to readable HTML pages.
"""

import json
import os
import subprocess
from pathlib import Path
from datetime import datetime
from zoneinfo import ZoneInfo
from html import escape
import markdown

# Project root
PROJECT_ROOT = Path(__file__).parent.parent
DOCS_DIR = PROJECT_ROOT / 'docs'
WEBSITE_DIR = PROJECT_ROOT / 'website'
OUTPUTS_DIR = DOCS_DIR / 'outputs'
SESSIONS_DIR = DOCS_DIR / 'sessions'


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


def extract_content_parts(content):
    """Extract text and tool calls from message content."""
    text_parts = []
    tool_calls = []

    if isinstance(content, str):
        return content, []
    elif isinstance(content, list):
        for item in content:
            if isinstance(item, dict):
                if item.get('type') == 'text':
                    text = item.get('text', '')
                    # Skip IDE notification blocks
                    if not text.startswith('<ide_opened_file>') and not text.startswith('<system-reminder>'):
                        text_parts.append(text)
                elif item.get('type') == 'tool_use':
                    tool_calls.append({
                        'name': item.get('name', 'unknown'),
                        'input': item.get('input', {}),
                        'id': item.get('id', '')
                    })
            elif isinstance(item, str):
                text_parts.append(item)
        return '\n'.join(text_parts), tool_calls
    return '', []


def format_timestamp(timestamp_str):
    """Format ISO timestamp to human-readable format in NZ time."""
    try:
        dt = datetime.fromisoformat(timestamp_str.replace('Z', '+00:00'))
        nz_dt = dt.astimezone(ZoneInfo('Pacific/Auckland'))
        return nz_dt.strftime('%Y-%m-%d %H:%M:%S')
    except:
        return timestamp_str


def parse_jsonl(file_path):
    """Parse JSONL file and extract messages."""
    messages = []

    with open(file_path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue

            try:
                entry = json.loads(line)

                # Skip queue operations and summaries for now
                if entry.get('type') in ['queue-operation']:
                    continue

                # Handle summary entries
                if entry.get('type') == 'summary':
                    messages.append({
                        'role': 'summary',
                        'content': entry.get('summary', ''),
                        'timestamp': None
                    })
                    continue

                # Handle regular messages
                if 'message' in entry:
                    msg = entry['message']
                    role = msg.get('role', entry.get('type', 'unknown'))
                    content_parts = msg.get('content', '')
                    content, tool_calls = extract_content_parts(content_parts)
                    timestamp = entry.get('timestamp')

                    # Check for tool results in the message
                    tool_results = {}
                    if isinstance(content_parts, list):
                        for item in content_parts:
                            if isinstance(item, dict) and item.get('type') == 'tool_result':
                                tool_use_id = item.get('tool_use_id')
                                result_content = item.get('content', '')
                                # Also check for toolUseResult field in the entry
                                if 'toolUseResult' in entry:
                                    result_data = entry['toolUseResult']
                                    if 'stdout' in result_data or 'stderr' in result_data:
                                        # Bash output
                                        result_content = result_data.get('stdout', '') or result_data.get('stderr', '')
                                tool_results[tool_use_id] = result_content

                    # Skip messages with no content and no tool calls (e.g., only IDE notifications)
                    if not content and not tool_calls and not tool_results:
                        continue

                    messages.append({
                        'role': role,
                        'content': content,
                        'tool_calls': tool_calls,
                        'tool_results': tool_results,
                        'timestamp': timestamp,
                        'error': entry.get('error'),
                        'uuid': entry.get('uuid')
                    })

            except json.JSONDecodeError as e:
                print(f"Warning: Failed to parse line in {file_path}: {e}")
                continue

    return messages


def generate_chatlog_html(session_id, session_name, messages, github_url):
    """Generate HTML page for a chatlog."""
    source_url = f"{github_url}/blob/main/docs/outputs/{session_id}.jsonl"

    # First pass: build map of tool calls to their results
    tool_results_map = {}
    for msg in messages:
        tool_results = msg.get('tool_results', {})
        tool_results_map.update(tool_results)

    html = f'''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{escape(session_name)} - ClankerOS</title>
    <link rel="stylesheet" href="../css/style.css">
</head>
<body>
    <div class="container">
        <h1><a href="../index.html">ClankerOS</a></h1>
        <h2>{escape(session_name)}</h2>
        <p class="meta">
            <a href="../chatlogs.html">← Back to chatlogs</a> |
            <a href="{escape(source_url)}">View source on GitHub</a>
        </p>

        <div class="chatlog-container">
'''

    message_counter = 0
    for msg in messages:
        role = msg['role']
        content = msg['content']
        tool_calls = msg.get('tool_calls', [])
        tool_results = msg.get('tool_results', {})
        timestamp = msg.get('timestamp')
        error = msg.get('error')

        # Skip messages that only contain tool results (they're displayed with the tool call)
        if not content and not tool_calls and tool_results:
            continue

        message_counter += 1
        message_id = f'msg-{message_counter}'

        role_class = role.replace('_', '-')
        role_display = role.replace('_', ' ').title()

        html += f'                <div class="message {escape(role_class)}" id="{message_id}">\n'
        html += f'                    <div class="message-header">\n'
        html += f'                        <a href="#{message_id}" class="message-anchor">#</a>\n'
        html += f'                        {escape(role_display)}\n'

        if timestamp:
            html += f'                        <span class="message-timestamp">{escape(format_timestamp(timestamp))}</span>\n'

        html += f'                    </div>\n'

        if content:
            # Convert markdown to HTML for message content
            md = markdown.Markdown(extensions=['fenced_code', 'tables', 'sane_lists'])
            content_html = md.convert(content)
            html += f'                    <div class="message-content">{content_html}</div>\n'

        # Add tool calls as collapsible sections
        for tool_call in tool_calls:
            tool_name = tool_call['name']
            tool_input = tool_call['input']

            # Get the result for this tool call if available
            tool_result = tool_results_map.get(tool_call['id'], '')

            # Format common tools nicely
            if tool_name == 'Bash':
                command = tool_input.get('command', '')
                desc = tool_input.get('description', '')
                html += f'                    <details class="tool-call tool-call-compact">\n'
                html += f'                        <summary>\n'
                html += f'                            <span class="tool-icon">⚙️</span>\n'
                html += f'                            <code>{escape(command)}</code>\n'
                if desc:
                    html += f'                            <span class="tool-desc">{escape(desc)}</span>\n'
                html += f'                        </summary>\n'
                if tool_result:
                    html += f'                        <pre><code>{escape(tool_result)}</code></pre>\n'
                html += f'                    </details>\n'

            elif tool_name == 'Write':
                file_path = tool_input.get('file_path', '')
                content = tool_input.get('content', '')
                html += f'                    <details class="tool-call tool-call-compact">\n'
                html += f'                        <summary>\n'
                html += f'                            <span class="tool-icon">📝</span>\n'
                html += f'                            <span>Writing to <code>{escape(file_path)}</code></span>\n'
                html += f'                        </summary>\n'
                if content:
                    html += f'                        <pre><code>{escape(content)}</code></pre>\n'
                html += f'                    </details>\n'

            elif tool_name == 'Edit':
                file_path = tool_input.get('file_path', '')
                old_string = tool_input.get('old_string', '')
                new_string = tool_input.get('new_string', '')
                html += f'                    <details class="tool-call tool-call-compact">\n'
                html += f'                        <summary>\n'
                html += f'                            <span class="tool-icon">✏️</span>\n'
                html += f'                            <span>Editing <code>{escape(file_path)}</code></span>\n'
                html += f'                        </summary>\n'
                if old_string or new_string:
                    html += f'                        <div class="edit-diff">\n'
                    if old_string:
                        html += f'                            <div class="diff-old"><strong>Old:</strong><pre><code>{escape(old_string)}</code></pre></div>\n'
                    if new_string:
                        html += f'                            <div class="diff-new"><strong>New:</strong><pre><code>{escape(new_string)}</code></pre></div>\n'
                    html += f'                        </div>\n'
                html += f'                    </details>\n'

            elif tool_name == 'Read':
                file_path = tool_input.get('file_path', '')
                html += f'                    <div class="tool-call-inline">\n'
                html += f'                        <span class="tool-icon">👁️</span>\n'
                html += f'                        <span>Reading <code>{escape(file_path)}</code></span>\n'
                html += f'                    </div>\n'

            elif tool_name == 'TodoWrite':
                todos = tool_input.get('todos', [])
                html += f'                    <details class="tool-call tool-call-compact">\n'
                html += f'                        <summary>\n'
                html += f'                            <span class="tool-icon">✓</span>\n'
                html += f'                            <span>Updating todo list ({len(todos)} items)</span>\n'
                html += f'                        </summary>\n'
                if todos:
                    html += f'                        <div style="padding: 1rem;">\n'
                    html += f'                            <ul style="list-style: none; padding: 0; margin: 0;">\n'
                    for todo in todos:
                        status = todo.get('status', 'pending')
                        content = todo.get('content', '')
                        checked = 'checked disabled' if status == 'completed' else 'disabled'
                        style = 'text-decoration: line-through; color: #666;' if status == 'completed' else ''
                        badge_color = '#4caf50' if status == 'completed' else ('#2196f3' if status == 'in_progress' else '#999')
                        badge_text = 'completed' if status == 'completed' else ('in progress' if status == 'in_progress' else 'pending')
                        html += f'                                <li style="margin-bottom: 0.5rem;">\n'
                        html += f'                                    <input type="checkbox" {checked} style="margin-right: 0.5rem;">\n'
                        html += f'                                    <span style="{style}">{escape(content)}</span>\n'
                        html += f'                                    <span style="font-size: 0.8rem; color: {badge_color}; margin-left: 0.5rem;">({badge_text})</span>\n'
                        html += f'                                </li>\n'
                    html += f'                            </ul>\n'
                    html += f'                        </div>\n'
                html += f'                    </details>\n'

            elif tool_name == 'Glob':
                pattern = tool_input.get('pattern', '')
                html += f'                    <div class="tool-call-inline">\n'
                html += f'                        <span class="tool-icon">🔍</span>\n'
                html += f'                        <span>Finding files: <code>{escape(pattern)}</code></span>\n'
                html += f'                    </div>\n'

            elif tool_name == 'Grep':
                pattern = tool_input.get('pattern', '')
                html += f'                    <div class="tool-call-inline">\n'
                html += f'                        <span class="tool-icon">🔍</span>\n'
                html += f'                        <span>Searching for: <code>{escape(pattern)}</code></span>\n'
                html += f'                    </div>\n'

            else:
                # For other tools, show the full JSON in a collapsible section
                tool_input_json = json.dumps(tool_input, indent=2)
                html += f'                    <details class="tool-call">\n'
                html += f'                        <summary>Tool: {escape(tool_name)}</summary>\n'
                html += f'                        <pre><code>{escape(tool_input_json)}</code></pre>\n'
                html += f'                    </details>\n'

        if error:
            html += f'                    <div class="message-error">Error: {escape(error)}</div>\n'

        html += f'                </div>\n'

    html += '''        </div>
    </div>
</body>
</html>
'''

    return html


def generate_chatlog_index(chatlogs):
    """Generate index page for chatlogs."""
    html = '''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Chatlogs - ClankerOS</title>
    <link rel="stylesheet" href="css/style.css">
</head>
<body>
    <div class="container">
        <h1><a href="index.html">ClankerOS</a></h1>
        <h2>Chatlogs</h2>

        <ul>
'''

    for chatlog in sorted(chatlogs, key=lambda x: x['name']):
        name = chatlog['name']
        session_id = chatlog['id']
        message_count = chatlog['message_count']

        html += f'''            <li class="session-item">
                <a href="chatlogs/{escape(session_id)}.html">{escape(name)}</a>
                <span class="meta">({message_count} messages)</span>
            </li>
'''

    html += '''        </ul>
    </div>
</body>
</html>
'''

    return html


def generate_sessions_index(session_files):
    """Generate index page for session summaries."""
    html = '''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Sessions - ClankerOS</title>
    <link rel="stylesheet" href="css/style.css">
</head>
<body>
    <div class="container">
        <h1><a href="index.html">ClankerOS</a></h1>
        <h2>Sessions</h2>

        <ul>
'''

    for session_file in sorted(session_files):
        # Read first few lines to extract title
        with open(SESSIONS_DIR / session_file, 'r') as f:
            title = f.readline().strip('# \n')

        html += f'''            <li class="session-item">
                <a href="sessions/{escape(session_file.replace('.md', '.html'))}">{escape(title)}</a>
            </li>
'''

    html += '''        </ul>
    </div>
</body>
</html>
'''

    return html


def main():
    """Main function to generate website."""
    print("Generating ClankerOS website...")

    # Get GitHub URL
    github_url = get_github_url()
    print(f"GitHub URL: {github_url}")

    # Create output directories
    chatlogs_dir = WEBSITE_DIR / 'chatlogs'
    sessions_web_dir = WEBSITE_DIR / 'sessions'
    chatlogs_dir.mkdir(parents=True, exist_ok=True)
    sessions_web_dir.mkdir(parents=True, exist_ok=True)

    # Update homepage with GitHub URL
    homepage = WEBSITE_DIR / 'index.html'
    if homepage.exists():
        content = homepage.read_text()
        content = content.replace(
            'https://github.com/yourusername/clankeros',
            github_url
        )
        homepage.write_text(content)

    # Process JSONL chatlogs
    print("\nProcessing chatlogs...")
    chatlogs = []

    for jsonl_file in OUTPUTS_DIR.glob('*.jsonl'):
        # Skip empty files
        if jsonl_file.stat().st_size == 0:
            continue

        session_id = jsonl_file.stem

        # Skip agent files for now
        if session_id.startswith('agent-'):
            continue

        print(f"  Processing {session_id}...")
        messages = parse_jsonl(jsonl_file)

        if not messages:
            print(f"    Skipped (no messages)")
            continue

        # Use first user message as session name, or default
        session_name = f"Session {session_id[:8]}"
        for msg in messages:
            if msg['role'] == 'user' and msg['content']:
                # Use first line of first user message
                first_line = msg['content'].split('\n')[0][:100]
                if first_line:
                    session_name = first_line
                break

        # Generate HTML
        html = generate_chatlog_html(session_id, session_name, messages, github_url)
        output_file = chatlogs_dir / f"{session_id}.html"
        output_file.write_text(html)

        chatlogs.append({
            'id': session_id,
            'name': session_name,
            'message_count': len(messages)
        })

        print(f"    Generated {output_file.name} ({len(messages)} messages)")

    # Generate chatlog index
    if chatlogs:
        print("\nGenerating chatlog index...")
        chatlog_index = generate_chatlog_index(chatlogs)
        (WEBSITE_DIR / 'chatlogs.html').write_text(chatlog_index)

    # Process session summaries (convert markdown to HTML later)
    # For now, just create an index
    print("\nProcessing session summaries...")
    session_files = list(SESSIONS_DIR.glob('*.md'))
    if session_files:
        sessions_index = generate_sessions_index([f.name for f in session_files])
        (WEBSITE_DIR / 'sessions.html').write_text(sessions_index)

    print(f"\n✓ Website generated in {WEBSITE_DIR}")
    print(f"  - {len(chatlogs)} chatlogs")
    print(f"  - {len(session_files)} session summaries")


if __name__ == '__main__':
    main()
