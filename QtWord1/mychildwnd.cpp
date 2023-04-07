#include "mychildwnd.h"
#include <QFileInfo>
#include <QFile>
MyChildWnd::MyChildWnd()
{
    //子窗口关闭时销毁该类的对象
    setAttribute(Qt::WA_DeleteOnClose);
    beSaved=false;
}
void MyChildWnd::newDoc()
{
    //设置窗口编号
    static int wndSeqNum=1;
    //将当前打卡的文档名为"Word 文档 编号"的形式,编号在使用后自增1
    myCurDocPath=tr("Word 文档 %1").arg(wndSeqNum++);
    //设置窗口的标题,文档改动后在其名称后显示"*"号标识
    setWindowTitle(myCurDocPath+"[*]"+tr("- MyWord"));
    //自动发送更改信号
    connect(document(),&QTextDocument::contentsChanged,this,&MyChildWnd::docBeModified);
}
void MyChildWnd::docBeModified()
{
    //判断文档是否修改
    setWindowModified(document()->isModified());
}
QString MyChildWnd::getCurDocName()
{
    //返回文件名字
    return QFileInfo(myCurDocPath).fileName();
}
bool MyChildWnd::loadDoc(const QString& docName)
{
    if(!docName.isEmpty())
    {
        if(!QFile::exists(docName)) return false;
        QFile doc(docName);
        if(!doc.open(QFile::ReadOnly)) return false;
        QByteArray text=doc.readAll();
        QTextCodec *text_codec=Qt::codecForHtml(text);
        QString str=text_codec->toUnicode(text);
        if(Qt::mightBeRichText(str))
        {
            this->setHtml(str);
        }else{
            str=QString::fromLocal8Bit(text);
            this->setPlainText(str);
        }
        setCurDoc(docName);
        connect(document(),&QTextDocument::contentsChanged,this,&MyChildWnd::docBeModified);
        return true;
    }else{
        return false;
    }
}
void MyChildWnd::setCurDoc(const QString &docName)
{
    myCurDocPath=QFileInfo(docName).canonicalFilePath();
    beSaved=true;//文档被修改过
    document()->setModified(false);//文档未被修改
    setWindowModified(false); //窗口不显示被改动表示
    setWindowTitle(getCurDocName()+"[*]");
}
bool MyChildWnd::saveDoc()
{
    if(!beSaved) return saveAsDoc();
    else return saveDocOpt(myCurDocPath);
}

bool MyChildWnd::saveAsDoc()
{
    QString docName=QFileDialog::getSaveFileName(this,tr("另存为"),myCurDocPath,
             tr("HTML 文档(*.htm *.html);;所有文件(*.*)"));
    if(docName.isEmpty()) return false;
    else return saveDocOpt(docName);
}

bool MyChildWnd::saveDocOpt(QString docName)
{
    if(!(docName.endsWith(".htm",Qt::CaseInsensitive)||docName.endsWith(".html",Qt::CaseInsensitive)))
    {
        docName+=".html";
    }
    QTextDocumentWriter writer(docName);
    bool success=writer.write(this->document());
    if(success) setCurDoc(docName);
    return success;
}

bool MyChildWnd::promptSave()
{
    if(!document()->isModified()) return true;
    QMessageBox::StandardButton result;
    result=QMessageBox::warning(this,tr("MyWord"),tr("文档'%1'已被更改,保存吗?").arg(getCurDocName()),
           QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel);
    if(result==QMessageBox::Save) return saveAsDoc();
    else if(result==QMessageBox::Cancel) return false;
    return true;
}

void MyChildWnd::closeEvent(QCloseEvent* event)
{
    if(promptSave())
    {
        event->accept();
    }else{
        event->ignore();
    }
}

void MyChildWnd::setFormatOnSelectedWord(const QTextCharFormat &fmt)
{
    QTextCursor tcursor=this->textCursor();
    if(!tcursor.hasSelection())
        tcursor.select(QTextCursor::WordUnderCursor);
    tcursor.mergeCharFormat(fmt);
    this->mergeCurrentCharFormat(fmt);
}

void MyChildWnd::setAlignOfDocumentText(int aligntype)
{
    if(aligntype==1) this->setAlignment(Qt::AlignLeft|Qt::AlignAbsolute);
    else if(aligntype==2) this->setAlignment(Qt::AlignHCenter);
    else if(aligntype==3) this->setAlignment(Qt::AlignRight|Qt::AlignAbsolute);
    else if(aligntype==4) this->setAlignment(Qt::AlignJustify);
}

void MyChildWnd::setParaStyle(int pstyle)
{
    QTextCursor tcursor = this->textCursor();
    if (pstyle != 0)
    {
        QTextListFormat::Style sname = QTextListFormat::ListDisc;
        switch (pstyle)
        {
            default:
            case 1:
                sname = QTextListFormat::ListDisc;
                break;
            case 2:
                sname = QTextListFormat::ListCircle;
                break;
            case 3:
                sname = QTextListFormat::ListSquare;
                break;
            case 4:
                sname = QTextListFormat::ListDecimal;
                break;
            case 5:
                sname = QTextListFormat::ListLowerAlpha;
                break;
            case 6:
                sname = QTextListFormat::ListUpperAlpha;
                break;
            case 7:
                sname = QTextListFormat::ListLowerRoman;
                break;
            case 8:
                sname = QTextListFormat::ListUpperRoman;
                break;
        }
        tcursor.beginEditBlock();
        QTextBlockFormat tBlockFmt = tcursor.blockFormat();
        QTextListFormat tListFmt;
        if (tcursor.currentList())
        {
            tListFmt = tcursor.currentList()->format();//#include <QtWidgets>
        } else {
            tListFmt.setIndent(tBlockFmt.indent() + 1);
            tBlockFmt.setIndent(0);
            tcursor.setBlockFormat(tBlockFmt);
        }
        tListFmt.setStyle(sname);
        tcursor.createList(tListFmt);
        tcursor.endEditBlock();
    } else {
        QTextBlockFormat tbfmt;
        tbfmt.setObjectIndex(-1);
        tcursor.mergeBlockFormat(tbfmt);
    }
}
