import sys
import json
import time

def main():
    if len(sys.argv) < 2:
        print("Usage: python stt_processor.py <audio_file_path>", file=sys.stderr)
        sys.exit(1)

    audio_file_path = sys.argv[1]
    print(f"Processing audio file: {audio_file_path}", file=sys.stderr)

    # Simulate STT processing time
    time.sleep(2)

    # Mock STT output with timestamps (simplified for now)
    # In a real scenario, this would come from an STT engine
    mock_lyrics_with_timestamps = [
        {"text": "[Intro]", "start_time": 0.0, "end_time": 3.0},
        {"text": "(Señor Narrátõ): Deuce mucho grañdé badass señors,", "start_time": 3.5, "end_time": 7.0},
        {"text": "Los Fiero e la "bringin' suxy back! Into you. Out. And back in again, yes!"", "start_time": 7.5, "end_time": 13.0},
        {"text": "Cumbriá Sir's and her's, we roll,", "start_time": 13.5, "end_time": 16.0},
        {"text": "What the fuck they doin in South America, lawl!", "start_time": 16.5, "end_time": 19.0},
        {"text": "[Tuba and horn riffs over dramatic whistle].", "start_time": 19.5, "end_time": 23.0},
        {"text": "[Verse]", "start_time": 24.0, "end_time": 25.0},
        {"text": "(Juan): Down dusty Colombian roads,", "start_time": 25.5, "end_time": 28.0},
        {"text": "Deuce lõcõs fiérõ a story told.", "start_time": 28.5, "end_time": 31.0},
        {"text": "Samuel L Jackson gringo négrõ, brutally bold,", "start_time": 31.5, "end_time": 35.0},
        {"text": "Blows off Cartel pinchérãtã heads, young and Banda old.", "start_time": 35.5, "end_time": 39.0},
        {"text": "(Sãmuél (Samuel L Jackson clone): "Danny, why you suxy, tight man pussy holed, badass Motherfucka'— YOU CAN FLY?".", "start_time": 39.5, "end_time": 46.0},
        {"text": "[Horn riffs/stabs over tuba sexto]", "start_time": 46.5, "end_time": 49.0},
        {"text": "[Pre-chorus][Building intensity and tension, mode change]", "start_time": 50.0, "end_time": 53.0},
        {"text": "This narco plane pilot you know, makes every Señotita moan,", "start_time": 53.5, "end_time": 58.0},
        {"text": "(Danny - Erotic Swagger): "I am Danny Steel, fly plane cabroñé, dontchya know?!" (So fly! Ahh-la-la-laaa, Ohh-si-si-siii)", "start_time": 58.5, "end_time": 66.0},
        {"text": "[Car Engine Starting]", "start_time": 66.5, "end_time": 68.0},
        {"text": "His Lap-dance is Juarez gold, "Sãmuél, we taking off! Are all weapons stowed?".", "start_time": 68.5, "end_time": 74.0},
        {"text": ""We comin' bold— 2 motherfuckers... in badass mode!"", "start_time": 74.5, "end_time": 78.0},
        {"text": "[Dramatic whistling riffs, tension peak]", "start_time": 78.5, "end_time": 81.0},
        {"text": "[Chrous][All vocalists harmony as band erupts]:", "start_time": 82.0, "end_time": 84.0},
        {"text": "(Quartet): 2 motherfuckin' hombres, bring sexy back, leave cartel death— "Vaminos!",", "start_time": 84.5, "end_time": 90.0},
        {"text": "Samuel Jackson Fiero Negrõ, Dãnny Steèl Picanto beunos.", "start_time": 90.5, "end_time": 94.0},
        {"text": "(Sãmuél - Blunt, Menacing): "Juan sum big black big white cock wit dat hambergausa Royale e Queso? Cartel dick ass fuck-oh's!".", "start_time": 94.5, "end_time": 101.0},
        {"text": "Pinchis putos!", "start_time": 101.5, "end_time": 102.5},
        {"text": "[Gunshot]", "start_time": 103.0, "end_time": 103.5},
        {"text": "[Helicopter]", "start_time": 104.0, "end_time": 104.5},
        {"text": "[Bomb Explosion]", "start_time": 105.0, "end_time": 105.5},
        {"text": "[Whiste Melody]", "start_time": 106.0, "end_time": 107.0},
        {"text": "*Ahh-la-la-la-laaa*", "start_time": 107.5, "end_time": 109.5},
        {"text": "*Ohh-si-si-si-siii*", "start_time": 110.0, "end_time": 112.0},
        {"text": "[Verse][Band relaxes gallop romance]", "start_time": 113.0, "end_time": 115.0},
        {"text": "(Juan): Tijuana they gank,", "start_time": 115.5, "end_time": 117.0},
        {"text": "All of Don Juan Cabroné cocaínà,", "start_time": 117.5, "end_time": 120.0},
        {"text": "Sãmuèl a finishing shank,", "start_time": 120.5, "end_time": 122.5},
        {"text": "Danny fully automatico cock fajita.", "start_time": 123.0, "end_time": 126.0},
        {"text": "Heads explode,", "start_time": 126.5, "end_time": 127.5},
        {"text": "Hombres groaned!", "start_time": 128.0, "end_time": 129.0},
        {"text": "Their fury sowed,", "start_time": 129.5, "end_time": 130.5},
        {"text": "Ass bloody mowed", "start_time": 131.0, "end_time": 132.0},
        {"text": "Danny consoles the surviving Chicas,", "start_time": 132.5, "end_time": 134.5},
        {"text": ""A lap-dance free? You lucky Señoritas!"", "start_time": 135.0, "end_time": 138.0},
        {"text": "[Whistle melody over sexto, tuba, brass, snare rolls]", "start_time": 138.5, "end_time": 142.0},
        {"text": "[Pre-chorus][Dark, menacing, tension builds]", "start_time": 143.0, "end_time": 145.0},
        {"text": "Tales flow wild thru Guadalajara", "start_time": 145.5, "end_time": 148.0},
        {"text": "Y sus mujeres chingara", "start_time": 148.5, "end_time": 150.0},
        {"text": "El Jèfé pattied, Don Juans ass fucked, life snuffed.", "start_time": 150.5, "end_time": 154.5},
        {"text": "Into you, in Méhecõ tough, and kinky rough.", "start_time": 155.0, "end_time": 158.0},
        {"text": "(Ooo-la-la!)", "start_time": 158.5, "end_time": 159.5},
        {"text": "(Sãmuél): "Next stop, Ciudad Juárez, heard El Serpiente thinks he rough?"", "start_time": 160.0, "end_time": 165.0},
        {"text": "(Danny): "Daddy Samuel L, unlike you, not fuckin tough enough!"", "start_time": 165.5, "end_time": 169.0},
        {"text": "[Chrous][All vocalists harmony as band erupts]", "start_time": 170.0, "end_time": 172.0},
        {"text": "(Group): 2 motherfuckin hombres, bring sexy back, leave cartel death— "Vaminos!",", "start_time": 172.5, "end_time": 178.0},
        {"text": "Samuel Jackson Fiero Negrõ, Dãnny Steèl Picanto beunos.", "start_time": 178.5, "end_time": 182.0},
        {"text": "(Sãmuél): "Juan sum big black big white cock wit dat Royale e Queso? Cartel DICKWADS!!".", "start_time": 182.5, "end_time": 189.0},
        {"text": "Pinchis putos!.", "start_time": 189.5, "end_time": 190.5},
        {"text": "[Whistle melody, Norteño beat]", "start_time": 191.0, "end_time": 193.0},
        {"text": "[Breakdown][tuba pulse, whistle, snare rolls]", "start_time": 194.0, "end_time": 196.0},
        {"text": "(Juan): Ciudad Juárez burnin', El Sèrpienté's lair,", "start_time": 196.5, "end_time": 200.0},
        {"text": "Sãmuél's Machete red, bloody fuckin' dispair.", "start_time": 200.5, "end_time": 203.5},
        {"text": "(Danny): “Chicas, watch Daddy dance. Stand safely— yes there”", "start_time": 204.0, "end_time": 207.5},
        {"text": "(Sãmuél): “Motherfucka, you die! Who cares!?”", "start_time": 208.0, "end_time": 210.0},
        {"text": "*Gunshot*", "start_time": 210.5, "end_time": 211.0},
        {"text": "(Quartet): Dos hombres fieros, cartel's end near.", "start_time": 211.5, "end_time": 214.5},
        {"text": "*Ah-la-la-laa*.", "start_time": 215.0, "end_time": 216.5},
        {"text": "[Outro]", "start_time": 217.0, "end_time": 217.5},
        {"text": "(Quartet): Dos motherfuckin hombres, sexy's back, cartels death— “Vaminos!” (Now fuck off!)", "start_time": 218.0, "end_time": 222.0},
        {"text": "[Gunshot]", "start_time": 222.5, "end_time": 223.0},
        {"text": "[End].", "start_time": 223.5, "end_time": 224.0},
    ]
    print(json.dumps(mock_lyrics_with_timestamps))

if __name__ == "__main__":
    main()
