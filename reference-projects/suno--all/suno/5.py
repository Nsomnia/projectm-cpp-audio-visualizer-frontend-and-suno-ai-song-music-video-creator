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
    Fetches a single Suno song page, uses regex to find the clip data,
    and downloads the assets. If it fails, it saves the HTML for debugging.
    """
    print(f"\nProcessing URL: {url}")
    try:
        response = session.get(url, timeout=15)
        response.raise_for_status()
    except requests.exceptions.RequestException as e:
        print(f"  ❌ Error fetching URL: {e}")
        return

    match = re.search(r'"clip":({.*?}),"persona":', response.text)

    if not match:
        # --- ENHANCED DEBUGGING ---
        # If the data isn't found, save the HTML we received to a file.
        print("  ❌ Could not find song data using regex. The page structure may have changed again.")
        song_id = url.strip().split('/')[-1]
        debug_filename = f"debug_FAIL_{sanitize_filename(song_id)}.html"
        with open(debug_filename, 'w', encoding='utf-8') as f:
            f.write(response.text)
        print(f"  ℹ️  DIAGNOSTIC: Saved the HTML that caused the failure to '{debug_filename}'")
        print(f"  ➡️  Please open this file and paste its contents in the chat for analysis.")
        return
        
    try:
        clip_data = json.loads(match.group(1))
        
        title = clip_data.get('title', 'untitled')
        audio_url = clip_data.get('audio_url')
        lyrics = clip_data.get('metadata', {}).get('prompt', '') or clip_data.get('metadata', {}).get('lyrics', '')

        if not audio_url:
            print("  ❌ Audio URL not found in the extracted data.")
            return

        print(f"  🎵 Found song: '{title}'")

        safe_title = sanitize_filename(title)
        audio_path = pathlib.Path(OUTPUT_DIR) / f"{safe_title}.mp3"
        lyrics_path = pathlib.Path(OUTPUT_DIR) / f"{safe_title}.txt"
        
        # Download Audio if it doesn't exist
        if not audio_path.exists():
            print(f"  📥 Downloading audio...")
            audio_response = session.get(audio_url, timeout=30)
            audio_response.raise_for_status()
            with open(audio_path, 'wb') as f:
                f.write(audio_response.content)
            print(f"  ✅ Audio saved to: {audio_path}")
        else:
            print(f"  ✅ Audio already exists.")

        # Save Lyrics if they don't exist
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
        # Process only the first 5 URLs for a quick test
        for url in urls[:5]:
            process_suno_url(url, session)

    print("\n\n--- Diagnostic run finished ---")
    print("If any 'debug_FAIL' files were created, please provide the contents of one of them.")


if __name__ == "__main__":
    main()