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
    # Replace slashes with dashes and remove other invalid characters
    filename = filename.replace('/', '-').replace('\\', '-')
    return re.sub(r'[?:"<>|]', "", filename)

def process_suno_url(url, session):
    """
    Fetches a single Suno song page, uses regex to find the clip data,
    and downloads the assets.
    """
    print(f"\nProcessing URL: {url}")
    try:
        response = session.get(url, timeout=15)
        response.raise_for_status()
    except requests.exceptions.RequestException as e:
        print(f"  ❌ Error fetching URL: {e}")
        return

    # --- NEW, REGEX-BASED METHOD ---
    # We search the raw HTML text for the JSON object containing the clip data.
    # This pattern looks for "clip":{...}, stopping at the ,"persona" that follows.
    match = re.search(r'"clip":({.*?}),"persona":', response.text)

    if not match:
        print("  ❌ Could not find song data using regex. The page structure may have changed again.")
        return
        
    try:
        # The first group captured by the regex is the JSON object string.
        clip_data = json.loads(match.group(1))
        
        title = clip_data.get('title', 'untitled')
        audio_url = clip_data.get('audio_url')
        lyrics = clip_data.get('metadata', {}).get('prompt', '')
        if not lyrics: # Fallback in case the key is named 'lyrics'
             lyrics = clip_data.get('metadata', {}).get('lyrics', '')

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
        print("Please create it and paste your Suno URLs inside, one per line.")
        return

    with open(URL_FILE, 'r') as f:
        urls = [line.strip() for line in f if line.strip()]

    if not urls:
        print(f"The file '{URL_FILE}' is empty. No songs to process.")
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