import re
import json
import pathlib
import requests
from playwright.sync_api import sync_playwright
import time

# --- Configuration ---
URL_FILE = "suno_urls.txt"
OUTPUT_DIR = "suno_downloads"

# --- ACTION REQUIRED: VERIFY THIS PATH ---
# This is the path to your browser's user data directory.
# This allows Playwright to use your existing cookies and login sessions.
# For Arch Linux with Chromium, this path is usually correct.
# ~ expands to your home directory.
CHROME_USER_DATA_DIR = "~/.config/chromium"

def sanitize_filename(filename):
    """Removes characters that are invalid for filenames."""
    filename = filename.replace('/', '-').replace('\\', '-')
    return re.sub(r'[?:"<>|]', "", filename)

def process_suno_url(url, page, session):
    """Uses a pre-launched page to get the HTML, then downloads assets."""
    print(f"\nProcessing URL: {url}")
    try:
        # Navigate to the URL. A real browser window will show this.
        page.goto(url, timeout=60000, wait_until='domcontentloaded')
        # Give the page an extra moment to settle and run its scripts
        time.sleep(3) 
        html_content = page.content()
        
    except Exception as e:
        print(f"  ❌ Error loading page with Playwright: {e}")
        return

    match = re.search(r'"clip":({.*?is_public":\w+})', html_content, re.DOTALL)

    if not match:
        print("  ❌ Could not find song data in the rendered HTML.")
        return
        
    try:
        clip_data = json.loads(match.group(1))
        
        title = clip_data.get('title', 'untitled')
        audio_url = clip_data.get('audio_url')
        metadata = clip_data.get('metadata', {})
        lyrics = metadata.get('prompt', '') or metadata.get('lyrics', '')

        if not audio_url:
            print("  ❌ Audio URL not found in the extracted data.")
            return

        print(f"  🎵 Found song: '{title}'")

        safe_title = sanitize_filename(title)
        audio_path = pathlib.Path(OUTPUT_DIR) / f"{safe_title}.mp3"
        lyrics_path = pathlib.Path(OUTPUT_DIR) / f"{safe_title}.txt"
        
        # Use the simple requests session to download files
        if not audio_path.exists():
            print(f"  📥 Downloading audio...")
            audio_response = session.get(audio_url, timeout=30)
            audio_response.raise_for_status()
            with open(audio_path, 'wb') as f:
                f.write(audio_response.content)
            print(f"  ✅ Audio saved to: {audio_path}")
        else:
            print(f"  ✅ Audio already exists.")

        if not lyrics_path.exists():
            if lyrics:
                print(f"  📝 Saving lyrics...")
                with open(lyrics_path, 'w', encoding='utf-8') as f:
                    f.write(lyrics)
                print(f"  ✅ Lyrics saved to: {lyrics_path}")
            else:
                print("  🟡 No lyrics found for this song.")
        else:
            print(f"  ✅ Lyrics already exist.")

    except (json.JSONDecodeError, KeyError, AttributeError) as e:
        print(f"  ❌ Error parsing song data: {e}")

def main():
    """Main function to set up Playwright and process URLs."""
    pathlib.Path(OUTPUT_DIR).mkdir(exist_ok=True)
    user_data_path = pathlib.Path(CHROME_USER_DATA_DIR).expanduser()

    if not user_data_path.exists():
        print(f"❌ ERROR: The user data directory was not found at '{user_data_path}'")
        print("Please find your Chromium/Chrome profile directory and update the CHROME_USER_DATA_DIR variable.")
        return
        
    if not pathlib.Path(URL_FILE).is_file():
        print(f"Error: The file '{URL_FILE}' was not found.")
        return

    with open(URL_FILE, 'r') as f:
        urls = [line.strip() for line in f if line.strip()]

    print(f"Found {len(urls)} song(s) to process from '{URL_FILE}'.")
    
    with sync_playwright() as p:
        # --- THE FIX: LAUNCH WITH A PERSISTENT CONTEXT ---
        print(f"🚀 Launching browser using your user profile at: {user_data_path}")
        context = p.chromium.launch_persistent_context(
            user_data_dir=user_data_path,
            headless=False,  # MUST be False to use a persistent context properly
            executable_path='/usr/bin/chromium'
        )
        page = context.new_page()
        
        with requests.Session() as session:
            for url in urls:
                process_suno_url(url, page, session)
        
        print("\nClosing browser in 10 seconds...")
        time.sleep(10)
        context.close()

    print("\n\n🎉 --- All songs processed! --- 🎉")

if __name__ == "__main__":
    main()