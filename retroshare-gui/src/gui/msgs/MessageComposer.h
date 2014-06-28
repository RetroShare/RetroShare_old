/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2007, RetroShare Team
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

#ifndef _MESSAGECOMPOSER_H
#define _MESSAGECOMPOSER_H

#include <QMainWindow>
#include <retroshare/rstypes.h>
#include <retroshare/rsmsgs.h>
#include "ui_MessageComposer.h"

class QAction;
class QComboBox;
class QFontComboBox;
class QTextEdit;
class QTextCharFormat;
class RSTreeWidgetItemCompareRole;
struct MessageInfo;

class MessageComposer : public QMainWindow 
{
    Q_OBJECT

public:
    enum enumType { TO, CC, BCC };
    enum enumMessageType { NORMAL, REPLY, FORWARD };
    enum destinationType { PEER_TYPE_SSL, PEER_TYPE_GROUP, PEER_TYPE_GXS };

public:
    /** Default Constructor */

    MessageComposer(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~MessageComposer();

    static void msgFriend(const RsPeerId &id);
    static void msgGxsIdentity(const RsGxsId& gxs_id) ;
    static void msgGroup(const std::string& group_id) ;

    static QString recommendMessage();
    static void recommendFriend(const std::list <RsPeerId> &sslIds, const RsPeerId &to = RsPeerId(), const QString &msg = "", bool autoSend = false);
    static void sendConnectAttemptMsg(const RsPgpId &gpgId, const RsPeerId &sslId, const QString &sslName);

    static MessageComposer *newMsg(const RsMessageId &msgId = RsMessageId());
    static MessageComposer *replyMsg(const RsMessageId &msgId, bool all);
    static MessageComposer *forwardMsg(const RsMessageId &msgId);

    /* worker fns */
    void  setFileList(const std::list<DirDetails>&);
    void  setFileList(const std::list<FileInfo>&);
    void  addFile(const FileInfo &fileInfo);
    void  setTitleText(const QString &title, enumMessageType type = NORMAL);
    void  setQuotedMsg(const QString &msg, const QString &header);
    void  setMsgText(const QString &msg, bool asHtml = false);
    void  addRecipient(enumType type, const RsPeerId &id);
    void  addRecipient(enumType type, const RsGxsId &gxs_id) ;
    void  addRecipient(enumType type, const std::string& group_id) ;

public slots:
    /* actions to take.... */
    void  sendMessage();
    void  cancelMessage();
    void  addImage();
    
    void changeFormatType(int styleIndex );
	 void updateSigningButton(int) ;

protected:
    void closeEvent (QCloseEvent * event);
    bool eventFilter(QObject *obj, QEvent *ev);

private slots:
    /* toggle Contacts DockWidget */
    void contextMenuFileList(QPoint);
    void contextMenuMsgSendList(QPoint);
    void pasteRecommended();
    void on_contactsdockWidget_visibilityChanged(bool visible);
    void toggleContacts();
    void buildCompleter();

    void fileNew();
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();
    void filePrint();
    void saveasDraft();

    //void filePrintPreview();
    void filePrintPdf();

    void textBold();
    void textUnderline();
    void textItalic();
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void textStyle(int styleIndex);
    void textColor();
    void textAlign(QAction *a);
    void smileyWidget();
    void addSmileys();

    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();

    void clipboardDataChanged();

    void fileHashingStarted();
    void fileHashingFinished(QList<HashedFile> hashedFiles);

    void attachFile();

    void fontSizeIncrease();
    void fontSizeDecrease();
    void blockQuote();
    void toggleCode();
    void addPostSplitter();

    void titleChanged();

    // Add to To/Cc/Bcc address fields
    void toggleShowNonFriend(bool bValue);
    void addTo();
    void addCc();
    void addBcc();
    void addRecommend();
    void editingRecipientFinished();
    void friendDetails();

    void peerStatusChanged(const QString& peer_id, int status);

    void tagAboutToShow();
    void tagSet(int tagId, bool set);
    void tagRemoveAll();

private:
    static QString buildReplyHeader(const MessageInfo &msgInfo);

    void processSettings(bool bLoad);

    void addContact(enumType type);
    void setTextColor(const QColor& col) ;
    void setupFileActions();
    void setupEditActions();
    void setupViewActions();
    void setupInsertActions();
    void setupFormatActions();

    bool load(const QString &f);
    bool maybeSave();
    //bool image_extension( QString nametomake );
    void setCurrentFileName(const QString &fileName);

    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void alignmentChanged(Qt::Alignment a);

    bool sendMessage_internal(bool bDraftbox);

    void calculateTitle();
    void addEmptyRecipient();

    bool getRecipientFromRow(int row, enumType &type, destinationType& dest_type, std::string &id);
    void setRecipientToRow(int row, enumType type, destinationType dest_type,const std::string &id);

    void clearTagLabels();
    void showTagLabels();

    QAction *actionSave,
    *actionAlignLeft,
    *actionAlignCenter,
    *actionAlignRight,
    *actionAlignJustify,
    *actionUndo,
    *actionRedo,
    *actionCut,
    *actionCopy,
    *actionPaste;

    QAction *contactSidebarAction;

    QTreeView *channelstreeView;

    QString fileName;
    QString nametomake;

    QColor codeBackground;
    QTextCharFormat defaultCharFormat;

    QHash<QString, QString> autoLinkDictionary;
    QHash<QString, QString> autoLinkTitleDictionary;
    QHash<QString, int> autoLinkTargetDictionary;

    RsMessageId m_msgParentId; // parent message id
    RsMessageId m_sDraftMsgId; // existing message id
    enumMessageType m_msgType;
    std::list<uint32_t> m_tagIds;
    QList<QLabel*> tagLabels;

    // needed to send system flags with reply
    unsigned msgFlags;

    RSTreeWidgetItemCompareRole *m_compareRole;
    QCompleter *m_completer;

    /** Qt Designer generated object */
    Ui::MessageComposer ui;

    std::list<FileInfo> _recList ;
    std::set<RsGxsId> _distant_peers ;	// we keep a list of them, in order to know which peer is a GXS id.
};

#endif
