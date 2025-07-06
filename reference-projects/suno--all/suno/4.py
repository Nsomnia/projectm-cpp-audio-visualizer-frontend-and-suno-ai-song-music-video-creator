import os
import re
import json
import pathlib
import requests

# --- Configuration ---
URL_FILE = "suno_urls.txt"
OUTPUT_DIR = "suno_downloads"

def sanitize_filename(filename):
    """Removes characters that are invalid for filenames."""
    filename = filename.replace('/', '-').replace('\\', '-')
    return re.sub(r'[?:"<>|]', "", filename)

def process_suno_url(url, session):
    """
    Fetches a Suno song page and downloads its assets using a robust regex
    that works for both audio and video pages.
    """
    print(f"\nProcessing URL: {url}")
    try:
        response = session.get(url, timeout=15)
        response.raise_for_status()
    except requests.exceptions.RequestException as e:
        print(f"  ❌ Error fetching URL: {e}")
        return

    # --- THE FINAL, ROBUST REGEX ---
    # This pattern looks for the 'clip' object and captures everything up to
    # the 'is_public' flag, which is present on both audio and video pages.
    # The re.DOTALL flag allows '.' to match newlines, making it more robust.
    match = re.search(r'"clip":({.*?is_public":\w+})', response.text, re.DOTALL)

    if not match:
        print("  ❌ Could not find song data. This page might be an exception.")
        song_id = url.strip().split('/')[-1]
        debug_filename = f"debug_FINAL_FAIL_{sanitize_filename(song_id)}.html"
        with open(debug_filename, 'w', encoding='utf-8') as f:
            f.write(response.text)
        print(f"  ℹ️  Saved the HTML that caused the failure to '{debug_filename}' for review.")
        return
        
    try:
        clip_data = json.loads(match.group(1))
        
        title = clip_data.get('title', 'untitled')
        audio_url = clip_data.get('audio_url')
        
        # The prompt/lyrics are stored in 'metadata'
        metadata = clip_data.get('metadata', {})
        lyrics = metadata.get('prompt', '') or metadata.get('lyrics', '')

        if not audio_url:
            print("  ❌ Audio URL not found in the extracted data.")
            return

        print(f"  🎵 Found song: '{title}'")

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
            print(f"  ✅ Audio saved to: {audio_path}")
        else:
            print(f"  ✅ Audio already exists.")

        # Save Lyrics
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
    """Main function to read URLs and process them."""
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
    
    with requests.Session() as session:
        session.headers.update({
            "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36"
        })
        for url in urls:
            process_suno_url(url, session)

    print("\n\n🎉 --- All songs processed! --- 🎉")

if __name__ == "__main__":
    main()