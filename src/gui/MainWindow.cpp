#include "MainWindow.h"
#include "Renderer.h"
#include <QScreen>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QWidget>
#include <QKeyEvent>
#include <QFileInfo>
#include <QDir>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialog>
#include <QLabel>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QToolBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QComboBox>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include "core/Config.h"

MainWindow::MainWindow(bool use_default_preset, const std::string& artist, const std::string& url, const std::string& fontPath, QWidget *parent)
    : QMainWindow(parent), m_renderer(nullptr), m_renderWindow(nullptr), m_ffmpegProcess(nullptr), m_isRecording(false), m_currentLyricIndex(0), m_sttProcess(nullptr)
{
    setupUi();
    setupQueueDock();

    m_renderWindow = new QWindow();
    m_renderWindow->setSurfaceType(QWindow::OpenGLSurface);
    QSurfaceFormat format;
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    m_renderWindow->setFormat(format);

    QWidget *renderWidget = QWidget::createWindowContainer(m_renderWindow);
    setCentralWidget(renderWidget);

    m_renderer = new Renderer(m_renderWindow, use_default_preset, artist, url, fontPath, menuBar()->height());

    // Connect the lyrics timer to periodically check audio position for lyric advancement
    connect(&m_lyricsTimer, &QTimer::timeout, this, &MainWindow::advanceLyrics);
    m_lyricsTimer.setInterval(100); // Check every 100ms for lyric updates

    m_sttProcess = new QProcess(this);
    connect(m_sttProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::processSttOutput);
    connect(m_sttProcess, &QProcess::errorOccurred, this, &MainWindow::onSttError);
}

MainWindow::~MainWindow() {
    if (m_sttProcess && m_sttProcess->state() == QProcess::Running) {
        m_sttProcess->terminate();
        m_sttProcess->waitForFinished();
    }
    delete m_sttProcess;
}

void MainWindow::setupUi()
{
    this->setWindowTitle("Aurora Visualizer");
    this->resize(1280, 720);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *openAction = new QAction(tr("&Open Audio File..."), this);
    fileMenu->addAction(openAction);
    connect(openAction, &QAction::triggered, this, &MainWindow::openAudioFile);

    QAction *addFilesAction = new QAction(tr("&Add Files to Queue..."), this);
    fileMenu->addAction(addFilesAction);
    connect(addFilesAction, &QAction::triggered, this, &MainWindow::addFilesToQueue);

    QAction *viewRecordingsAction = new QAction(tr("&View Recordings"), this);
    fileMenu->addAction(viewRecordingsAction);
    connect(viewRecordingsAction, &QAction::triggered, this, &MainWindow::viewRecordings);

    QAction *openLyricsAction = new QAction(tr("&Open Lyrics File..."), this);
    fileMenu->addAction(openLyricsAction);
    connect(openLyricsAction, &QAction::triggered, this, &MainWindow::openLyricsFile);

    QMenu *settingsMenu = menuBar()->addMenu(tr("&Settings"));
    QAction *openSettingsAction = new QAction(tr("&Open Settings..."), this);
    settingsMenu->addAction(openSettingsAction);
    connect(openSettingsAction, &QAction::triggered, this, &MainWindow::openSettings);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAction = new QAction(tr("&About..."), this);
    helpMenu->addAction(aboutAction);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::openAbout);

    QToolBar *toolBar = addToolBar(tr("Tools"));
    m_playPauseAction = new QAction(tr("Play"), this);
    toolBar->addAction(m_playPauseAction);
    connect(m_playPauseAction, &QAction::triggered, this, &MainWindow::togglePlayback);

    m_startRecordingAction = new QAction(tr("Record"), this);
    toolBar->addAction(m_startRecordingAction);
    connect(m_startRecordingAction, &QAction::triggered, this, &MainWindow::startRecording);

    m_stopRecordingAction = new QAction(tr("Stop"), this);
    toolBar->addAction(m_stopRecordingAction);
    m_stopRecordingAction->setEnabled(false);
    connect(m_stopRecordingAction, &QAction::triggered, this, &MainWindow::stopRecording);

    QAction *deleteBrokenPresetAction = new QAction(tr("Delete Broken Preset"), this);
    toolBar->addAction(deleteBrokenPresetAction);
    connect(deleteBrokenPresetAction, &QAction::triggered, this, &MainWindow::deleteBrokenPreset);

    QAction *downloadFromSunoAction = new QAction(tr("Download from Suno"), this);
    toolBar->addAction(downloadFromSunoAction);
    connect(downloadFromSunoAction, &QAction::triggered, this, &MainWindow::downloadFromSuno);

    m_recordingIndicator = new QLabel(tr("Not Recording"));
    statusBar()->addWidget(m_recordingIndicator);
}

void MainWindow::setupQueueDock()
{
    m_songQueueDock = new QDockWidget(tr("Song Queue"), this);
    m_songQueueDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget *dockWidgetContent = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;

    m_songQueueList = new QListWidget;
    m_songQueueList->setDragDropMode(QAbstractItemView::InternalMove);
    layout->addWidget(m_songQueueList);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *addButton = new QPushButton(tr("Add"));
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addFilesToQueue);
    buttonLayout->addWidget(addButton);

    QPushButton *removeButton = new QPushButton(tr("Remove"));
    connect(removeButton, &QPushButton::clicked, this, &MainWindow::removeSelectedFromQueue);
    buttonLayout->addWidget(removeButton);

    QPushButton *clearButton = new QPushButton(tr("Clear"));
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearQueue);
    buttonLayout->addWidget(clearButton);

    layout->addLayout(buttonLayout);

    QHBoxLayout *playbackButtons = new QHBoxLayout;
    QPushButton *prevButton = new QPushButton(tr("Previous"));
    connect(prevButton, &QPushButton::clicked, this, &MainWindow::playPreviousInQueue);
    playbackButtons->addWidget(prevButton);

    QPushButton *nextButton = new QPushButton(tr("Next"));
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::playNextInQueue);
    playbackButtons->addWidget(nextButton);

    layout->addLayout(playbackButtons);

    dockWidgetContent->setLayout(layout);
    m_songQueueDock->setWidget(dockWidgetContent);
    addDockWidget(Qt::RightDockWidgetArea, m_songQueueDock);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_N) {
        if (m_renderer) {
            m_renderer->selectRandomPreset();
        }
    } else if (event->key() == Qt::Key_P) {
        if (m_renderer) {
            m_renderer->selectPreviousPreset();
        }
    } else if (event->key() == Qt::Key_Space) {
        togglePlayback();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_isRecording) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Recording in Progress"), tr("A recording is currently in progress. Are you sure you want to exit?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No) {
            event->ignore();
            return;
        }
        stopRecording();
    }
    event->accept();
}

void MainWindow::openAudioFile()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        tr("Open Audio File"), "", tr("Audio Files (*.mp3 *.wav *.flac)"));

    if (!filePath.isEmpty()) {
        m_renderer->reset();
        m_lyrics.clear(); // Clear existing lyrics
        m_currentLyricIndex = 0;
        m_lyricsTimer.stop(); // Stop the timer until new lyrics are loaded
        m_renderer->clearLyrics(); // Clear displayed lyrics

        if (!m_renderer->getAudioEngine()->loadFile(filePath.toStdString())) {
            QMessageBox::critical(this, tr("Error"), tr("Could not load the selected audio file."));
        } else {
            QFileInfo fileInfo(filePath);
            m_renderer->setSongTitle(fileInfo.baseName().toStdString());

            // Start STT process for the loaded audio file
            QStringList args;
            args << "scripts/stt_processor.py" << filePath;
            m_sttProcess->start("python3", args);
        }
    }
}

void MainWindow::addFilesToQueue()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(this,
        tr("Add Audio Files to Queue"), "", tr("Audio Files (*.mp3 *.wav *.flac)"));

    for (const QString& filePath : filePaths) {
        if (m_songQueueList->findItems(filePath, Qt::MatchExactly).isEmpty()) {
            m_songQueueList->addItem(filePath);
        }
    }
}

void MainWindow::removeSelectedFromQueue()
{
    qDeleteAll(m_songQueueList->selectedItems());
}

void MainWindow::playNextInQueue()
{
    int currentIndex = m_songQueueList->currentRow();
    if (m_songQueueList->count() == 0) {
        return;
    }

    int nextIndex = (currentIndex + 1) % m_songQueueList->count();
    m_songQueueList->setCurrentRow(nextIndex);

    QString filePath = m_songQueueList->item(nextIndex)->text();
    m_renderer->reset();
    if (!m_renderer->getAudioEngine()->loadFile(filePath.toStdString())) {
        QMessageBox::critical(this, tr("Error"), tr("Could not load the selected audio file."));
    } else {
        QFileInfo fileInfo(filePath);
        m_renderer->setSongTitle(fileInfo.baseName().toStdString());
    }
}

void MainWindow::playPreviousInQueue()
{
    int currentIndex = m_songQueueList->currentRow();
    if (m_songQueueList->count() == 0) {
        return;
    }

    int prevIndex = (currentIndex - 1 + m_songQueueList->count()) % m_songQueueList->count();
    m_songQueueList->setCurrentRow(prevIndex);

    QString filePath = m_songQueueList->item(prevIndex)->text();
    m_renderer->reset();
    if (!m_renderer->getAudioEngine()->loadFile(filePath.toStdString())) {
        QMessageBox::critical(this, tr("Error"), tr("Could not load the selected audio file."));
    } else {
        QFileInfo fileInfo(filePath);
        m_renderer->setSongTitle(fileInfo.baseName().toStdString());
    }
}

void MainWindow::clearQueue()
{
    m_songQueueList->clear();
}

void MainWindow::openSettings()
{
    QDialog settingsDialog(this);
    settingsDialog.setWindowTitle(tr("Settings"));

    QTabWidget *tabWidget = new QTabWidget(&settingsDialog);
    QFormLayout form(&settingsDialog);
    form.addRow(tabWidget);

    // General Settings
    QWidget *generalTab = new QWidget();
    QFormLayout generalLayout(generalTab);
    Config config;
    QLineEdit *fontPathEdit = new QLineEdit(config.fontPath());
    generalLayout.addRow(tr("Font Path:"), fontPathEdit);
    QLineEdit *fontSizeEdit = new QLineEdit(QString::number(config.fontSize()));
    generalLayout.addRow(tr("Font Size:"), fontSizeEdit);
    tabWidget->addTab(generalTab, "General");

    // Video Settings
    QWidget *videoTab = new QWidget();
    QFormLayout videoLayout(videoTab);
    QComboBox *resolutionCombo = new QComboBox();
    resolutionCombo->addItems({"1920x1080", "1280x720", "1080x1920 (Mobile)", "720x1280 (Mobile)"});
    videoLayout.addRow(tr("Resolution:"), resolutionCombo);
    QLineEdit *bitrateEdit = new QLineEdit("8000k");
    videoLayout.addRow(tr("Bitrate:"), bitrateEdit);
    tabWidget->addTab(videoTab, "Video");

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &settingsDialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &settingsDialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &settingsDialog, &QDialog::reject);

    if (settingsDialog.exec() == QDialog::Accepted) {
        QSettings settings("config.ini", QSettings::IniFormat);
        settings.setValue("Font/path", fontPathEdit->text());
        settings.setValue("Font/size", fontSizeEdit->text().toInt());
        settings.setValue("Video/resolution", resolutionCombo->currentText());
        settings.setValue("Video/bitrate", bitrateEdit->text());
    }
}

void MainWindow::openAbout()
{
    QMessageBox::about(this, tr("About Aurora Visualizer"),
        tr("<h2>Aurora Visualizer</h2>"
           "<p>A highly-customizable audio visualizer.</p>"
           "<p>For more information, visit:"
           "<br><a href='https://github.com/Nsomnia/projectm-music-visualizer-music-video-auto-generator-suno'>https://github.com/Nsomnia/projectm-music-visualizer-music-video-auto-generator-suno</a></p>"
           "<h3>Keyboard Shortcuts:</h3>"
           "<ul>"
           "<li><b>N:</b> Next Preset</li>"
           "<li><b>P:</b> Previous Preset</li>"
           "<li><b>Space:</b> Play/Pause</li>"
           "</ul>"));
}

void MainWindow::startRecording()
{
    if (m_songQueueList->currentItem() == nullptr) {
        QMessageBox::warning(this, tr("No Song Selected"), tr("Please select a song from the queue to record."));
        return;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
    QString fileName = QString("video-%1.mp4").arg(timestamp);

    m_ffmpegProcess = new QProcess(this);
    connect(m_ffmpegProcess, &QProcess::errorOccurred, this, &MainWindow::onRecordingError);

    QSettings settings("config.ini", QSettings::IniFormat);
    QString resolution = settings.value("Video/resolution", "1280x720").toString();
    QString bitrate = settings.value("Video/bitrate", "8000k").toString();

    QStringList args;
    args << "-y" << "-f" << "x11grab" << "-s" << resolution
         << "-i" << QGuiApplication::primaryScreen()->name() << "-r" << "30"
         << "-b:v" << bitrate << fileName;

    m_ffmpegProcess->start("ffmpeg", args);
    m_isRecording = true;
    m_recordingIndicator->setText(tr("Recording..."));
    m_startRecordingAction->setEnabled(false);
    m_stopRecordingAction->setEnabled(true);
    togglePlayback();
}

void MainWindow::stopRecording()
{
    if (m_ffmpegProcess && m_ffmpegProcess->state() == QProcess::Running) {
        m_ffmpegProcess->terminate();
        m_ffmpegProcess->waitForFinished();
    }
    m_isRecording = false;
    m_recordingIndicator->setText(tr("Not Recording"));
    m_startRecordingAction->setEnabled(true);
    m_stopRecordingAction->setEnabled(false);
    if (m_renderer->getAudioEngine()->isPlaying()) {
        togglePlayback();
    }
}

void MainWindow::togglePlayback()
{
    if (m_renderer->getAudioEngine()->isPlaying()) {
        m_renderer->getAudioEngine()->pause();
        m_playPauseAction->setText(tr("Play"));
    } else {
        if (m_songQueueList->currentItem() == nullptr && m_songQueueList->count() > 0) {
            m_songQueueList->setCurrentRow(0);
            playNextInQueue();
        }
        m_renderer->getAudioEngine()->play();
        m_playPauseAction->setText(tr("Pause"));
    }
}

void MainWindow::onRecordingError(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart) {
        QMessageBox::critical(this, tr("Recording Error"), tr("Failed to start FFmpeg. Make sure it's installed and in your PATH."));
    } else {
        QMessageBox::critical(this, tr("Recording Error"), tr("An error occurred during recording."));
    }
    stopRecording();
}

void MainWindow::viewRecordings()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::currentPath()));
}

void MainWindow::deleteBrokenPreset()
{
    if (m_renderer->isPresetBroken()) {
        QString presetPath = QString::fromStdString(m_renderer->getCurrentPresetPath());
        QFile presetFile(presetPath);
        QDir backupDir("backups/broken_presets");
        if (!backupDir.exists()) {
            backupDir.mkpath(".");
        }
        QString newPath = backupDir.filePath(QFileInfo(presetPath).fileName());
        if (presetFile.rename(newPath)) {
            QMessageBox::information(this, tr("Preset Moved"), tr("The broken preset has been moved to backups/broken_presets."));
            m_renderer->selectNextPreset();
        } else {
            QMessageBox::warning(this, tr("Move Failed"), tr("Could not move the broken preset file."));
        }
    } else {
        QMessageBox::information(this, tr("No Broken Preset"), tr("The current preset is not broken."));
    }
}

void MainWindow::downloadFromSuno()
{
    bool ok;
    QString url = QInputDialog::getText(this, tr("Download from Suno"),
                                        tr("Suno.ai Song URL:"), QLineEdit::Normal,
                                        "", &ok);
    if (ok && !url.isEmpty()) {
        QProcess *sunoDownloader = new QProcess(this);

        QFile envFile(".env");
        if (envFile.open(QIODevice::ReadOnly)) {
            QTextStream in(&envFile);
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.startsWith("SUNO_COOKIE=")) {
                    QString cookie = line.mid(12);
                    sunoDownloader->setEnvironment(QProcess::systemEnvironment() << QString("SUNO_COOKIE=%1").arg(cookie));
                }
            }
        }

        connect(sunoDownloader, &QProcess::finished, this, [=](int exitCode, QProcess::ExitStatus exitStatus){
            if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                QMessageBox::information(this, tr("Download Complete"), tr("The song has been downloaded and added to the queue."));
                // Find the downloaded file and add it to the queue
                QDir dir(".");
                QStringList filters;
                filters << "*.mp3";
                dir.setNameFilters(filters);
                dir.setSorting(QDir::Time);
                QStringList list = dir.entryList();
                if (!list.isEmpty()) {
                    m_songQueueList->addItem(dir.absoluteFilePath(list.first()));
                }
            } else {
                QMessageBox::critical(this, tr("Download Failed"), tr("Could not download the song from the provided URL."));
            }
            sunoDownloader->deleteLater();
        });
        sunoDownloader->start("scripts/.venv/bin/python3", QStringList() << "scripts/suno_downloader.py" << url);
    }
}

void MainWindow::openLyricsFile()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        tr("Open Lyrics File"), "", tr("Text Files (*.txt);;All Files (*)"));

    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            std::string lyricsContent = in.readAll().toStdString();
            loadLyrics(lyricsContent);
            file.close();
            m_lyricsTimer.start(); // Start the lyrics timer when lyrics are loaded
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Could not open the selected lyrics file."));
        }
    }
}

void MainWindow::loadLyrics(const std::string& jsonLyricsContent)
{
    m_lyrics.clear();
    m_currentLyricIndex = 0;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(jsonLyricsContent).toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        QMessageBox::critical(this, tr("JSON Parse Error"), tr("Failed to parse STT output: ") + parseError.errorString());
        return;
    }

    if (doc.isArray()) {
        QJsonArray jsonArray = doc.array();
        for (const QJsonValue &value : jsonArray) {
            if (value.isObject()) {
                QJsonObject obj = value.toObject();
                LyricLine line;
                line.text = obj["text"].toString().toStdString();
                line.startTime = obj["start_time"].toDouble();
                line.endTime = obj["end_time"].toDouble();
                m_lyrics.push_back(line);
            }
        }
    }

    if (!m_lyrics.empty()) {
        m_lyricsTimer.start(); // Start the lyrics timer when lyrics are loaded
    }
}

void MainWindow::displayLyrics()
{
    if (m_lyrics.empty() || m_currentLyricIndex < 0 || m_currentLyricIndex >= m_lyrics.size()) {
        m_renderer->clearLyrics();
        return;
    }

    m_renderer->setLyrics(m_lyrics[m_currentLyricIndex].text);
}

void MainWindow::processSttOutput()
{
    QByteArray output = m_sttProcess->readAllStandardOutput();
    std::string jsonOutput = output.toStdString();
    loadLyrics(jsonOutput);
}

void MainWindow::onSttError(QProcess::ProcessError error)
{
    QMessageBox::critical(this, tr("STT Error"), tr("An error occurred during STT processing: ") + m_sttProcess->errorString());
    m_renderer->clearLyrics();
    m_lyrics.clear();
    m_lyricsTimer.stop();
}

void MainWindow::advanceLyrics()
{
    if (m_renderer->getAudioEngine()->isPlaying() && !m_lyrics.empty()) {
        double currentPlaybackTime = m_renderer->getAudioEngine()->getCurrentPosition(); // in seconds

        // Find the current lyric line based on playback time
        int newLyricIndex = -1;
        for (size_t i = 0; i < m_lyrics.size(); ++i) {
            if (currentPlaybackTime >= m_lyrics[i].startTime && currentPlaybackTime < m_lyrics[i].endTime) {
                newLyricIndex = i;
                break;
            }
        }

        if (newLyricIndex != m_currentLyricIndex) {
            m_currentLyricIndex = newLyricIndex;
            if (m_currentLyricIndex != -1) {
                displayLyrics();
            } else {
                m_renderer->clearLyrics(); // Clear if no lyric is active
            }
        }
    } else if (!m_renderer->getAudioEngine()->isPlaying()) {
        m_renderer->clearLyrics();
        m_currentLyricIndex = -1;
    }
}

