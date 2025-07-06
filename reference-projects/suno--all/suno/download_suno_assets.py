import re
import json
import pathlib
import requests
from playwright.sync_api import sync_playwright
import time

# --- Configuration ---
URL_FILE = "suno_urls.txt"  # This is no longer used but kept for reference
OUTPUT_DIR = "suno_downloads"
CHROME_USER_DATA_DIR = "~/.config/chromium"

# This dictionary will store all the song data we find.
# Using a dictionary with song ID as the key automatically handles duplicates.
ALL_SONGS_DATA = {}

def sanitize_filename(filename):
    """Removes characters that are invalid for filenames."""
    filename = filename.replace('/', '-').replace('\\', '-')
    return re.sub(r'[?:"<>|]', "", filename)

def handle_api_response(response):
    """This function is called for every network response the browser receives."""
    # We are only interested in the API call that fetches the song feed.
    if "/api/feed/" in response.url:
        print(f"[+] Intercepted API call: {response.url}")
        try:
            data = response.json()
            # The song data is in a list, usually named 'feed' or 'clips'
            clips = data.get('feed', data.get('clips', []))
            
            initial_count = len(ALL_SONGS_DATA)
            for clip in clips:
                # Store the entire clip object, keyed by its unique ID
                ALL_SONGS_DATA[clip['id']] = clip
            
            new_songs = len(ALL_SONGS_DATA) - initial_count
            if new_songs > 0:
                print(f"    -> Found {new_songs} new songs. Total unique songs: {len(ALL_SONGS_DATA)}")

        except Exception as e:
            print(f"    -> Could not process response as JSON: {e}")

def main():
    """Main function to launch Playwright, intercept API calls, and download assets."""
    pathlib.Path(OUTPUT_DIR).mkdir(exist_ok=True)
    user_data_path = pathlib.Path(CHROME_USER_DATA_DIR).expanduser()

    if not user_data_path.exists():
        print(f"❌ ERROR: Chromium user data directory not found at '{user_data_path}'")
        return

    with sync_playwright() as p:
        print(f"🚀 Launching browser using your profile: {user_data_path}")
        browser = p.chromium.launch(
            headless=False,
            executable_path='/usr/bin/chromium'
        )
        page = browser.new_page()

        # Set up the API response listener BEFORE navigating.
        page.on("response", handle_api_response)

        print("Navigating to your Suno library. Please wait...")
        page.goto("https://suno.com/me", timeout=90000, wait_until='domcontentloaded')

        print("Page loaded. Starting automatic scrolling to load all songs...")
        
        last_height = page.evaluate("document.body.scrollHeight")
        while True:
            page.evaluate("window.scrollTo(0, document.body.scrollHeight)")
            print("   -> Scrolled down...")
            time.sleep(3)  # Wait for new content to load
            new_height = page.evaluate("document.body.scrollHeight")
            if new_height == last_height:
                print("Reached the bottom of the library.")
                break
            last_height = new_height
        
        print(f"\n✅ Scraping complete. Found a total of {len(ALL_SONGS_DATA)} unique songs.")
        print("Closing browser and beginning download phase...")
        browser.close()

    # --- Download Phase ---
    if not ALL_SONGS_DATA:
        print("\nNo songs were found. Exiting.")
        return

    print("\n--- Starting Download Process ---")
    with requests.Session() as session:
        for song_id, clip_data in ALL_SONGS_DATA.items():
            title = clip_data.get('title', f"untitled_{song_id}")
            safe_title = sanitize_filename(title)
            print(f"\nProcessing: '{title}'")

            audio_url = clip_data.get('audio_url')
            lyrics = clip_data.get('metadata', {}).get('prompt', '') or clip_data.get('metadata', {}).get('lyrics', '')

            audio_path = pathlib.Path(OUTPUT_DIR) / f"{safe_title}.mp3"
            lyrics_path = pathlib.Path(OUTPUT_DIR) / f"{safe_title}.txt"

            if audio_url:
                if not audio_path.exists():
                    try:
                        print(f"  -> Downloading audio...")
                        audio_response = session.get(audio_url, timeout=30)
                        audio_response.raise_for_status()
                        with open(audio_path, 'wb') as f:
                            f.write(audio_response.content)
                        print(f"  ✅ Audio saved.")
                    except requests.exceptions.RequestException as e:
                        print(f"  ❌ Failed to download audio: {e}")
                else:
                    print(f"  ✅ Audio already exists.")
            
            if not lyrics_path.exists():
                if lyrics:
                    print(f"  -> Saving lyrics...")
                    with open(lyrics_path, 'w', encoding='utf-8') as f:
                        f.write(lyrics)
                    print(f"  ✅ Lyrics saved.")
            else:
                print(f"  ✅ Lyrics already exist.")

    print("\n\n🎉 --- All assets downloaded! --- 🎉")

if __name__ == "__main__":
    main()