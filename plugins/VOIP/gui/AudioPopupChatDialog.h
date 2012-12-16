#include <QObject>
#include <QGraphicsEffect>
#include <gui/SpeexProcessor.h>
#include <gui/chat/PopupChatDialog.h>
#include <gui/audiodevicehelper.h>

class QPushButton;

#define VOIP_SOUND_INCOMING_CALL "VOIP_incoming_call"

class AudioPopupChatDialog: public PopupChatDialog
{
	Q_OBJECT

	public:
		AudioPopupChatDialog(QWidget *parent = NULL); 

		virtual ~AudioPopupChatDialog()
		{
			if(inputDevice != NULL)
				inputDevice->stop() ;
		}

		void addAudioData(const QString name, QByteArray* array) ;
	private slots:
		void toggleAudioListen();
		void toggleAudioMuteCapture();

	public slots:
		void sendAudioData();

	protected:
		QAudioInput* inputDevice;
		QAudioOutput* outputDevice;
		QtSpeex::SpeexInputProcessor* inputProcessor;
		QtSpeex::SpeexOutputProcessor* outputProcessor;

		virtual void updateStatus(int status) ;

		QPushButton *audioListenToggleButton ;
		QPushButton *audioMuteCaptureToggleButton ;
};

