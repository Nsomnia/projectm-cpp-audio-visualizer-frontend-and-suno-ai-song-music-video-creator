import re
import json
import pathlib
import requests
from playwright.sync_api import sync_playwright
from playwright_stealth import stealth_sync
import time

# --- Configuration ---
URL_FILE = "suno_urls.txt"
OUTPUT_DIR = "suno_downloads"
CHROME_USER_DATA_DIR = "~/.config/chromium"

def sanitize_filename(filename):
    """Removes characters that are invalid for filenames."""
    filename = filename.replace('/', '-').replace('\\', '-')
    return re.sub(r'[?:"<>|]', "", filename)

def process_suno_url(url, page, session):
    """
    Cleans the URL, uses a stealthy Playwright page to get the HTML,
    then extracts and downloads the assets.
    """
    clean_url = url.split('?')[0]
    print(f"\nProcessing: {clean_url}")
    
    try:
        page.goto(clean_url, timeout=60000)
        
        # --- NEW, MORE ROBUST WAITING STRATEGY ---
        # Instead of waiting for network, we wait for a specific element: the song title.
        # This is more reliable than waiting for the network to be idle.
        page.wait_for_selector('h1', timeout=30000)
        
        # Give a brief moment for any final scripts to settle after the title appears
        time.sleep(1)
        
        html_content = page.content()
        
    except Exception as e:
        print(f"  ❌ Error loading page or finding selector: {e}")
        return

    # Use the robust regex on the fully-rendered HTML
    match = re.search(r'"clip":({.*?is_public":\w+})', html_content, re.DOTALL)

    if not match:
        print("  ❌ Could not find song data in the final HTML. The page structure may be unique.")
        return
        
    try:
        clip_data = json.loads(match.group(1))
        
        title = clip_data.get('title', 'untitled')
        audio_url = clip_data.get('audio_url')
        metadata = clip_data.get('metadata', {})
        lyrics = metadata.get('prompt', '') or metadata.get('lyrics', '')

        if not audio_url:
            print("  ❌ Audio URL not found in data.")
            return

        print(f"  🎵 Found: '{title}'")

        safe_title = sanitize_filename(title)
        audio_path = pathlib.Path(OUTPUT_DIR) / f"{safe_title}.mp3"
        lyrics_path = pathlib.Path(OUTPUT_DIR) / f"{safe_title}.txt"
        
        # Download Audio
        if not audio_path.exists():
            print(f"  📥 Downloading audio...")
            audio_response = session.get(audio_url, timeout=30)
            audio_response.raise_for_status()
            with open(audio_path, 'wb') as f:
                f.write(audio_response.content)
            print(f"  ✅ Audio saved.")
        else:
            print(f"  ✅ Audio already exists.")

        # Save Lyrics
        if not lyrics_path.exists():
            if lyrics:
                print(f"  📝 Saving lyrics...")
                with open(lyrics_path, 'w', encoding='utf-8') as f:
                    f.write(lyrics)
                print(f"  ✅ Lyrics saved.")
            else:
                print("  🟡 No lyrics found.")
        else:
            print(f"  ✅ Lyrics already exist.")

    except (json.JSONDecodeError, KeyError, AttributeError) as e:
        print(f"  ❌ Error parsing song data: {e}")

def main():
    """Main function to set up a stealthy Playwright instance and process URLs."""
    pathlib.Path(OUTPUT_DIR).mkdir(exist_ok=True)
    user_data_path = pathlib.Path(CHROME_USER_DATA_DIR).expanduser()

    if not user_data_path.exists():
        print(f"❌ ERROR: Chromium user data directory not found at '{user_data_path}'")
        return
        
    if not pathlib.Path(URL_FILE).is_file():
        print(f"Error: The file '{URL_FILE}' was not found.")
        return

    with open(URL_FILE, 'r') as f:
        urls = [line.strip() for line in f if line.strip()]

    print(f"Found {len(urls)} URLs to process from '{URL_FILE}'.")
    
    with sync_playwright() as p:
        print(f"🚀 Launching stealth-enabled browser...")
        browser = p.chromium.launch(
            headless=False,
            executable_path='/usr/bin/chromium'
        )
        page = browser.new_page()
        
        # Apply the stealth patches
        stealth_sync(page)
        
        with requests.Session() as session:
            session.headers.update({
                "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36"
            })
            for url in urls:
                process_suno_url(url, page, session)
        
        print("\nClosing browser in 5 seconds...")
        time.sleep(5)
        browser.close()

    print("\n\n🎉 --- Download phase complete! --- 🎉")

if __name__ == "__main__":
    main()