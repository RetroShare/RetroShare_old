/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (c) 2010, RetroShare Team
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#include <QMimeData>
#include <QTextDocumentFragment>
#include <QCompleter>
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QScrollBar>
#include <QMenu>

#include "MimeTextEdit.h"
#include "util/HandleRichText.h"
#include "gui/RetroShareLink.h"
#include "rshare.h"

#include <retroshare/rspeers.h>
#include <retroshare/rsversion.h>
#include <retroshare/rsiface.h>
#include <retroshare/rsplugin.h>
#include <microhttpd.h>

MimeTextEdit::MimeTextEdit(QWidget *parent)
    : RSTextEdit(parent), mCompleter(0)
{
	mCompleterKeyModifiers = Qt::ControlModifier;
	mCompleterKey = Qt::Key_Space;
	mForceCompleterShowNextKeyEvent = false;
}

bool MimeTextEdit::canInsertFromMimeData(const QMimeData* source) const
{
#if QT_VERSION >= 0x040700
	// embedded images are not supported before QT 4.7.0
	if (source != NULL) {
		if (source->hasImage()) {
			return true;
		}
	}
#endif

	return RSTextEdit::canInsertFromMimeData(source);
}

void MimeTextEdit::insertFromMimeData(const QMimeData* source)
{
#if QT_VERSION >= 0x040700
	// embedded images are not supported before QT 4.7.0
	if (source != NULL) {
		if (source->hasImage()) {
			// insert as embedded image
			QImage image = qvariant_cast<QImage>(source->imageData());
			if (image.isNull() == false) {
				QString	encodedImage;
				if (RsHtml::makeEmbeddedImage(image, encodedImage, 640*480)) {
					QTextDocumentFragment fragment = QTextDocumentFragment::fromHtml(encodedImage);
					textCursor().insertFragment(fragment);
					return;
				}
			}
		}
	}
#endif

	return RSTextEdit::insertFromMimeData(source);
}

void MimeTextEdit::setCompleter(QCompleter *completer)
{
	if (mCompleter)
		QObject::disconnect(mCompleter, 0, this, 0);

	mCompleter = completer;

	if (!mCompleter)
		return;

	mCompleter->setWidget(this);
	mCompleter->setCompletionMode(QCompleter::PopupCompletion);
	mCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	QObject::connect(mCompleter, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
}

QCompleter *MimeTextEdit::completer() const
{
	return mCompleter;
}

void MimeTextEdit::insertCompletion(const QString& completion)
{
	if (mCompleter->widget() != this)
		return;

	QTextCursor tc = textCursor();
	if (mCompleter->completionPrefix().length() > 0) {
		tc.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
	}
	tc.removeSelectedText();
	tc.insertText(mCompleterStartString+completion);
	mCompleterStartString.clear();
	setTextCursor(tc);
}

QString MimeTextEdit::textUnderCursor() const
{
	QTextCursor tc = textCursor();
	tc.select(QTextCursor::WordUnderCursor);
	return tc.selectedText();
}

void MimeTextEdit::focusInEvent(QFocusEvent *e)
{
	if (mCompleter)
		mCompleter->setWidget(this);

	RSTextEdit::focusInEvent(e);
}

void MimeTextEdit::keyPressEvent(QKeyEvent *e)
{
	if (mCompleter && mCompleter->popup()->isVisible()) {
		// The following keys are forwarded by the completer to the widget
		switch (e->key()) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Escape:
		case Qt::Key_Tab:
		case Qt::Key_Backtab:
			mCompleter->popup()->hide();
			mForceCompleterShowNextKeyEvent=false;
			e->ignore();
			return; // let the completer do default behavior
		default:
			break;
		}
	}

	bool isShortcut = ((e->modifiers() & mCompleterKeyModifiers) && e->key() == mCompleterKey);
	if (isShortcut && !mForceCompleterShowNextKeyEvent) {
		mCompleterStartString.clear();
	}
	isShortcut |= mForceCompleterShowNextKeyEvent;
	if (!mCompleter || !isShortcut) // do not process the shortcut when we have a completer
		RSTextEdit::keyPressEvent(e);

	if (!mCompleter) return; //Nothing else to do if not mCompleter initialized

	if (!isShortcut && (mCompleter && !mCompleter->popup()->isVisible())) {
		return;
	}

	if (!mForceCompleterShowNextKeyEvent) {
		static QString eow(" ~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
		if (!isShortcut && !e->text().isEmpty() && eow.contains(e->text())){
			mCompleter->popup()->hide();
			return;
		}
	}

	QString completionPrefix = textUnderCursor();
	if (completionPrefix != mCompleter->completionPrefix()) {
		mCompleter->setCompletionPrefix(completionPrefix);
		mCompleter->popup()->setCurrentIndex(mCompleter->completionModel()->index(0, 0));
	}

	QRect cr = cursorRect();
	cr.setWidth(mCompleter->popup()->sizeHintForColumn(0) + mCompleter->popup()->verticalScrollBar()->sizeHint().width());
	mCompleter->complete(cr); // popup it up!

	if (mCompleter->completionCount()==0 && isShortcut){
		RSTextEdit::keyPressEvent(e);// Process the key if no match
	}
	mForceCompleterShowNextKeyEvent = false;
}

void MimeTextEdit::setCompleterKeyModifiers(Qt::KeyboardModifier modifiers)
{
	mCompleterKeyModifiers = modifiers;
}

Qt::KeyboardModifier MimeTextEdit::getCompleterKeyModifiers() const
{
	return mCompleterKeyModifiers;
}

void MimeTextEdit::setCompleterKey(Qt::Key key)
{
	mCompleterKey = key;
}

Qt::Key MimeTextEdit::getCompleterKey() const
{
	return mCompleterKey;
}

void MimeTextEdit::forceCompleterShowNextKeyEvent(QString startString)
{
	if (!mCompleter) return; //Nothing else to do if not mCompleter initialized

	if(!mCompleter->popup()->isVisible()){
		mForceCompleterShowNextKeyEvent = true;
		mCompleterStartString = startString;
	}
}

void MimeTextEdit::addContextMenuAction(QAction *action)
{
	mContextMenuActions.push_back(action);
}

void MimeTextEdit::contextMenuEvent(QContextMenuEvent *e)
{
	emit calculateContextMenuActions();

	QMenu *contextMenu = createStandardContextMenu(e->pos());

	/* Add actions for pasting links */
	contextMenu->addSeparator();
	QAction *pasteLinkAction = contextMenu->addAction(QIcon(":/images/pasterslink.png"), tr("Paste RetroShare Link"), this, SLOT(pasteLink()));
	contextMenu->addAction(QIcon(":/images/pasterslink.png"), tr("Paste my certificate link"), this, SLOT(pasteOwnCertificateLink()));
	contextMenu->addAction(QIcon(":/images/pasterslink.png"), tr("Paste my Bug Reporting Info"), this, SLOT(pasteSysInfo()));

	if (RSLinkClipboard::empty()) {
		pasteLinkAction->setDisabled(true);
	}

	QList<QAction*>::iterator it;
	for (it = mContextMenuActions.begin(); it != mContextMenuActions.end(); ++it) {
		contextMenu->addAction(*it);
	}

	contextMenu->exec(QCursor::pos());

	delete(contextMenu);
}

void MimeTextEdit::pasteLink()
{
	insertHtml(RSLinkClipboard::toHtml()) ;
}

void MimeTextEdit::pasteOwnCertificateLink()
{
	RetroShareLink link;
	RsPeerId ownId = rsPeers->getOwnId();

	if (link.createCertificate(ownId)) {
		insertHtml(link.toHtml() + " ");
	}
}
static void addLibraries(MimeTextEdit *mTextEdit, const std::string &name, const std::list<RsLibraryInfo> &libraries)
{

	mTextEdit->insertHtml(QString::fromUtf8(name.c_str()));
	mTextEdit->insertHtml("<br>");

	std::list<RsLibraryInfo>::const_iterator libraryIt;
	for (libraryIt = libraries.begin(); libraryIt != libraries.end(); ++libraryIt) {;
		mTextEdit->insertHtml(QString::fromUtf8(libraryIt->mName.c_str()));
		mTextEdit->insertHtml("<br>");
		mTextEdit->insertHtml(QString::fromUtf8(libraryIt->mVersion.c_str()));
		mTextEdit->insertHtml("<br>");
		mTextEdit->insertHtml("<br>");
	}
}
void MimeTextEdit::pasteSysInfo()
{
	/*int major = RS_MAJOR_VERSION ;
	int minor = RS_MINOR_VERSION ;
	int build = RS_BUILD_NUMBER ;
	int svn_rev = RS_REVISION_NUMBER ;*/
	QString rsVerString = "RetroShare Version: ";
	rsVerString+=Rshare::retroshareVersion(true);
	//rsVerString.sprintf("RetroShare Version: %i.%i (%i, %i) <br>",major, minor, build,svn_rev);
	insertHtml(rsVerString);
	insertHtml("<br>");


#if QT_VERSION >= QT_VERSION_CHECK (5, 0, 0)
	//TODO use QT5s nice qsysinfo class!
#else
	#ifdef Q_WS_X11
	QString OS="Linux";
	#endif
	#ifdef Q_WS_WIN
	QString OS="Windows";
	#endif
	#ifdef Q_WS_MACX
	QString OS="Mac";
	#endif
	insertHtml(OS+" ");
#endif

	QString qtver = QString("QT ")+QT_VERSION_STR;
	insertHtml(qtver);
	insertHtml("<br>");

	/* Add version numbers of libretroshare */
	std::list<RsLibraryInfo> libraries;
	RsControl::instance()->getLibraries(libraries);
	addLibraries(this, "libretroshare", libraries);

	/* Add version numbers of RetroShare */
	// Add versions here. Find a better place.
	libraries.clear();
	libraries.push_back(RsLibraryInfo("Libmicrohttpd", MHD_get_version()));
	addLibraries(this, "RetroShare", libraries);

	/* Add version numbers of plugins */
	if (rsPlugins) {
		for (int i = 0; i < rsPlugins->nbPlugins(); ++i) {
			RsPlugin *plugin = rsPlugins->plugin(i);
			if (plugin) {
				libraries.clear();
				plugin->getLibraries(libraries);
				addLibraries(this, plugin->getPluginName(), libraries);
			}
		}
	}
}
