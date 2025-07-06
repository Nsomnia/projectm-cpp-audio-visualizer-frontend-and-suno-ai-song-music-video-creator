import requests

# --- Please paste one of your song URLs here ---
url = "https://suno.com/song/7cce556d-9afc-4b67-87c2-f1748bb6ee88"

# --- Leave the rest of the script as is ---
output_file = "debug_page_source.html"

print(f"Fetching URL: {url}")

headers = {
    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36"
}

try:
    response = requests.get(url, headers=headers, timeout=15)
    response.raise_for_status()
    
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(response.text)
        
    print(f"✅ Success! The HTML content has been saved to '{output_file}'.")
    print("Please copy the content of that file and paste it in the chat.")

except requests.exceptions.RequestException as e:
    print(f"❌ An error occurred: {e}")