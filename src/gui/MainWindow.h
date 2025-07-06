#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QDockWidget>
#include <QProcess>
#include <QLabel>
#include <QTimer>

class Renderer;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(bool use_default_preset, const std::string& artist, const std::string& url, const std::string& fontPath, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void openAudioFile();
    void addFilesToQueue();
    void removeSelectedFromQueue();
    void playNextInQueue();
    void playPreviousInQueue();
    void clearQueue();
    void openSettings();
    void openAbout();
    void startRecording();
    void stopRecording();
    void togglePlayback();
    void onRecordingError(QProcess::ProcessError error);
    void viewRecordings();
    void deleteBrokenPreset();
    void downloadFromSuno();
    void openLyricsFile();
    void advanceLyrics();
    void checkSongFinished();
    void nextPreset();
    void prevPreset();

private:
    void setupUi();
    void setupQueueDock();

    Renderer* m_renderer;
    QWindow* m_renderWindow;

    QListWidget* m_songQueueList;
    QDockWidget* m_songQueueDock;

    QProcess* m_ffmpegProcess;
    bool m_isRecording;
    QLabel* m_recordingIndicator;

    QAction* m_playPauseAction;
    QAction* m_startRecordingAction;
    QAction* m_stopRecordingAction;

    bool m_isPlaying;

    struct LyricLine {
        std::string text;
        double startTime;
        double endTime;
    };

    std::vector<LyricLine> m_lyrics;
    int m_currentLyricIndex;
    QTimer m_lyricsTimer;
    QTimer m_songEndTimer;
    QProcess* m_sttProcess;

    void loadLyrics(const std::string& jsonLyricsContent);
    void displayLyrics();
    void processSttOutput();
    void onSttError(QProcess::ProcessError error);
};
