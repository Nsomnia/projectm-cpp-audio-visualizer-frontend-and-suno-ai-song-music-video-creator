void MainWindow::checkSongFinished()
{
    if (m_isPlaying) {
        float currentPosition = m_renderer->getAudioEngine()->getCurrentPosition();
        float songDuration = m_renderer->getAudioEngine()->getSongDuration();

        if (songDuration > 0 && currentPosition >= songDuration) {
            playNextInQueue();
        }
    }
}