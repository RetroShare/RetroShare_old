#include <retroshare/rsplugin.h>
#include <util/rsversion.h>
#include <QTranslator>

#include "LinksCloudPlugin.h"
#include "LinksDialog.h"

static void *inited = new LinksCloudPlugin() ;

extern "C" {
	void *RETROSHARE_PLUGIN_provide()
	{
		static LinksCloudPlugin *p = new LinksCloudPlugin() ;

		return (void*)p ;
	}
	// This symbol contains the svn revision number grabbed from the executable. 
	// It will be tested by RS to load the plugin automatically, since it is safe to load plugins
	// with same revision numbers, assuming that the revision numbers are up-to-date.
	//
	uint32_t RETROSHARE_PLUGIN_revision = SVN_REVISION_NUMBER ;

	// This symbol contains the svn revision number grabbed from the executable. 
	// It will be tested by RS to load the plugin automatically, since it is safe to load plugins
	// with same revision numbers, assuming that the revision numbers are up-to-date.
	//
	uint32_t RETROSHARE_PLUGIN_api = RS_PLUGIN_API_VERSION ;
}

#define IMAGE_LINKS ":/images/irkick.png"

void LinksCloudPlugin::getPluginVersion(int& major,int& minor,int& svn_rev) const
{
	major = 5 ;
	minor = 4 ;
	svn_rev = SVN_REVISION_NUMBER ;
}

LinksCloudPlugin::LinksCloudPlugin()
{
	mRanking = NULL ;
	mainpage = NULL ;
        mIcon = NULL ;
        mPlugInHandler = NULL;
        mPeers = NULL;
        mFiles = NULL;
}

void LinksCloudPlugin::setInterfaces(RsPlugInInterfaces &interfaces){

    mPeers = interfaces.mPeers;
    mFiles = interfaces.mFiles;
}

MainPage *LinksCloudPlugin::qt_page() const
{
	if(mainpage == NULL)
                mainpage = new LinksDialog(mPeers, mFiles) ;

	return mainpage ;
}

RsCacheService *LinksCloudPlugin::rs_cache_service() const
{
	if(mRanking == NULL)
	{
                mRanking = new p3Ranking(mPlugInHandler) ; // , 3600 * 24 * 30 * 6); // 6 Months
		rsRanks = mRanking ;
	}

	return mRanking ;
}

void LinksCloudPlugin::setPlugInHandler(RsPluginHandler *pgHandler){
    mPlugInHandler = pgHandler;

}

QIcon *LinksCloudPlugin::qt_icon() const
{
	if(mIcon == NULL)
	{
		Q_INIT_RESOURCE(LinksCloud_images) ;

		mIcon = new QIcon(IMAGE_LINKS) ;
	}

	return mIcon ;
}

std::string LinksCloudPlugin::getShortPluginDescription() const
{
	return QApplication::translate("LinksCloudPlugin", "This plugin provides a set of cached links, and a voting system to promote them.").toUtf8().constData();
}

std::string LinksCloudPlugin::getPluginName() const
{
	return QApplication::translate("LinksCloudPlugin", "LinksCloud").toUtf8().constData();
}

QTranslator* LinksCloudPlugin::qt_translator(QApplication */*app*/, const QString& languageCode, const QString& externalDir) const
{
	if (languageCode == "en") {
		return NULL;
	}

	QTranslator* translator = new QTranslator();

	if (translator->load(externalDir + "/LinksCloud_" + languageCode + ".qm")) {
		return translator;
	} else if (translator->load(":/lang/LinksCloud_" + languageCode + ".qm")) {
		return translator;
	}

	delete(translator);
	return NULL;
}
