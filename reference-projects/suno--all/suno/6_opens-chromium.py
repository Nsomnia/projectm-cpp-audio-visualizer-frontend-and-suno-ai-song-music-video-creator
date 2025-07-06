import re
import json
import pathlib
import requests
from playwright.sync_api import sync_playwright

# --- Configuration ---
URL_FILE = "suno_urls.txt"
OUTPUT_DIR = "suno_downloads"

def sanitize_filename(filename):
    """Removes characters that are invalid for filenames."""
    filename = filename.replace('/', '-').replace('\\', '-')
    return re.sub(r'[?:"<>|]', "", filename)

def process_suno_url(url, page, session):
    """
    Uses Playwright to get the full HTML, then extracts and downloads assets.
    """
    print(f"\nProcessing URL: {url}")
    try:
        page.goto(url, timeout=60000)
        # Wait for a key element that indicates the page is loaded.
        # This selector targets the container holding the song's main content.
        page.wait_for_selector('div[data-suno-id]', timeout=30000)
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

    if not pathlib.Path(URL_FILE).is_file():
        print(f"Error: The file '{URL_FILE}' was not found.")
        return

    with open(URL_FILE, 'r') as f:
        urls = [line.strip() for line in f if line.strip()]

    if not urls:
        print(f"The file '{URL_FILE}' is empty.")
        return

    print(f"Found {len(urls)} song(s) to process from '{URL_FILE}'.")
    
    with sync_playwright() as p:
        # --- THE FIX FOR ARCH LINUX ---
        # This tells Playwright to use your system's installed Chromium
        # instead of the broken version it downloaded.
        print("🚀 Launching browser using system's executable path...")
        browser = p.chromium.launch(headless=True, executable_path='/usr/bin/chromium')
        
        page = browser.new_page()
        
        with requests.Session() as session:
            session.headers.update({
                "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36"
            })
            for url in urls:
                process_suno_url(url, page, session)
        
        browser.close()

    print("\n\n🎉 --- All songs processed! --- 🎉")

if __name__ == "__main__":
    main()