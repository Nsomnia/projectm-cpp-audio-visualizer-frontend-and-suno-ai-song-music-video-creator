import os
import re
import json
import pathlib
import requests
from bs4 import BeautifulSoup

# --- Configuration ---
# The name of the file containing your list of Suno URLs.
URL_FILE = "suno_urls.txt"
# The name of the folder where songs and lyrics will be saved.
OUTPUT_DIR = "suno_downloads"

def sanitize_filename(filename):
    """Removes characters that are invalid for filenames."""
    return re.sub(r'[\\/*?:"<>|]', "", filename)

def process_suno_url(url, session):
    """
    Fetches a single Suno song page, extracts the metadata, and downloads the assets.
    """
    print(f"\nProcessing URL: {url}")
    try:
        response = session.get(url, timeout=15)
        response.raise_for_status()  # Raises an HTTPError for bad responses (4xx or 5xx)
    except requests.exceptions.RequestException as e:
        print(f"  ❌ Error fetching URL: {e}")
        return

    soup = BeautifulSoup(response.text, 'html.parser')
    
    # Find the specific script tag containing the song's data.
    # In Next.js pages, this is often found in a script tag with JSON content.
    # We'll look for a key field like "audio_url" to identify the right script.
    data_script = None
    for script in soup.find_all('script'):
        if script.string and '"audio_url"' in script.string:
            data_script = script.string
            break

    if not data_script:
        print("  ❌ Could not find the data script on the page. The site structure may have changed.")
        return

    # Use regex to find the 'clip' JSON object within the script text.
    match = re.search(r'"clip":({.*?}),"persona"', data_script)
    if not match:
        print("  ❌ Could not extract the 'clip' JSON object from the script. Regex needs adjustment.")
        return
        
    try:
        clip_data = json.loads(match.group(1))
        
        title = clip_data.get('title', 'untitled')
        audio_url = clip_data.get('audio_url')
        lyrics = clip_data.get('metadata', {}).get('prompt', '')

        if not audio_url:
            print("  ❌ Audio URL not found in the extracted data.")
            return

        print(f"  🎵 Found song: '{title}'")

        # Sanitize title for file paths
        safe_title = sanitize_filename(title)
        
        # Define file paths
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
            print(f"  ✅ Audio already exists. Skipping download.")

        # Save Lyrics if they don't exist
        if not lyrics_path.exists():
            print(f"  📝 Saving lyrics...")
            with open(lyrics_path, 'w', encoding='utf-8') as f:
                f.write(lyrics)
            print(f"  ✅ Lyrics saved to: {lyrics_path}")
        else:
            print(f"  ✅ Lyrics already exist. Skipping save.")

    except (json.JSONDecodeError, AttributeError) as e:
        print(f"  ❌ Error parsing song data: {e}")


def main():
    """
    Main function to read URLs and process them.
    """
    # Create the output directory if it doesn't exist
    pathlib.Path(OUTPUT_DIR).mkdir(exist_ok=True)

    # Check if the URL file exists
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
    
    # Use a session object for connection pooling
    with requests.Session() as session:
        # Suno might expect standard browser headers
        session.headers.update({
            "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36"
        })
        for url in urls:
            process_suno_url(url, session)

    print("\n\n🎉 --- All songs processed! --- 🎉")


if __name__ == "__main__":
    main()