#include "vrepMainHeader.h"
#include "codeEditorContainer.h"
#include "pluginContainer.h"
#include "luaScriptFunctions.h"
#include "v_rep_internal.h"
#include "v_repStrings.h"
#include "app.h"

QString CCodeEditorContainer::getXmlColorString(const char* colTxt,const int rgbCol[3])
{
    return(getXmlColorString(colTxt,rgbCol[0],rgbCol[1],rgbCol[2]));
}

QString CCodeEditorContainer::getXmlColorString(const char* colTxt,int r,int g,int b)
{
    QString retVal=QString("%1=\"%2 %3 %4\"").arg(colTxt).arg(r).arg(g).arg(b);
    return(retVal);
}

QString CCodeEditorContainer::getKeywords(int scriptType,bool threaded)
{
    QString retVal="<keywords1>";
    retVal+=getFuncKeywords(scriptType,threaded);
    retVal+="</keywords1>";
    retVal+="<keywords2>";
    retVal+=getVarKeywords(scriptType,threaded);
    retVal+="</keywords2>";
    return(retVal);
}

QString CCodeEditorContainer::getFuncKeywords(int scriptType,bool threaded)
{
    QString retVal;
    std::vector<std::string> t;
    std::map<std::string,bool> map;
    pushAllVrepFunctionNamesThatStartSame_autoCompletionList("",t,map,scriptType,threaded);
    App::ct->luaCustomFuncAndVarContainer->pushAllFunctionNamesThatStartSame_autoCompletionList("",t,map);
    std::sort(t.begin(),t.end());
    for (size_t i=0;i<t.size();i++)
    {
        std::string tip(getVrepFunctionCalltip(t[i].c_str(),scriptType,threaded,true));
        if (tip.size()==0)
        {
            for (size_t j=0;j<App::ct->luaCustomFuncAndVarContainer->allCustomFunctions.size();j++)
            {
                std::string n=App::ct->luaCustomFuncAndVarContainer->allCustomFunctions[j]->getFunctionName();
                if (n.compare(t[i].c_str())==0)
                {
                    tip=App::ct->luaCustomFuncAndVarContainer->allCustomFunctions[j]->getCallTips();
                    break;
                }
            }
        }
        retVal+=QString("<item word=\"%1\" autocomplete=\"true\" calltip=\"%2\"/>").arg(t[i].c_str()).arg(tip.c_str());
    }
    return(retVal);
}

QString CCodeEditorContainer::getVarKeywords(int scriptType,bool threaded)
{
    QString retVal;
    std::vector<std::string> t;
    std::map<std::string,bool> map;
    pushAllVrepVariableNamesThatStartSame_autoCompletionList("",t,map);
    App::ct->luaCustomFuncAndVarContainer->pushAllVariableNamesThatStartSame_autoCompletionList("",t,map);
    std::sort(t.begin(),t.end());
    for (size_t i=0;i<t.size();i++)
        retVal+=QString("<item word=\"%1\" autocomplete=\"true\"/>").arg(t[i].c_str());
    return(retVal);
}

QString CCodeEditorContainer::translateXml(const char* oldXml,const char* callback)
{
    QString retVal("<editor ");
    if (strlen(callback)>0)
        retVal+=QString(" on-close=\"%1\"").arg(callback);
    if (oldXml!=nullptr)
    {
        tinyxml2::XMLDocument xmldoc;
        tinyxml2::XMLError error=xmldoc.Parse(oldXml);
        if(error==tinyxml2::XML_NO_ERROR)
        {
            tinyxml2::XMLElement* rootElement=xmldoc.FirstChildElement();
            const char* val=rootElement->Attribute("title");
            if (val!=nullptr)
                retVal+=QString(" title=\"%1\"").arg(val);
            val=rootElement->Attribute("editable");
            if (val!=nullptr)
                retVal+=QString(" editable=\"%1\"").arg(val);
            val=rootElement->Attribute("searchable");
            if (val!=nullptr)
                retVal+=QString(" searchable=\"%1\"").arg(val);
            val=rootElement->Attribute("tabWidth");
            if (val!=nullptr)
                retVal+=QString(" tab-width=\"%1\"").arg(val);
            val=rootElement->Attribute("textColor");
            if (val!=nullptr)
                retVal+=QString(" text-col=\"%1\"").arg(val);
            val=rootElement->Attribute("backgroundColor");
            if (val!=nullptr)
                retVal+=QString(" background-col=\"%1\"").arg(val);
            val=rootElement->Attribute("selectionColor");
            if (val!=nullptr)
                retVal+=QString(" selection-col=\"%1\"").arg(val);
            val=rootElement->Attribute("size");
            if (val!=nullptr)
                retVal+=QString(" size=\"%1\"").arg(val);
            val=rootElement->Attribute("position");
            if (val!=nullptr)
                retVal+=QString(" position=\"%1\"").arg(val);
            val=rootElement->Attribute("isLua");
            if (val!=nullptr)
                retVal+=QString(" is-lua=\"%1\"").arg(val);
            val=rootElement->Attribute("commentColor");
            if (val!=nullptr)
                retVal+=QString(" comment-col=\"%1\"").arg(val);
            val=rootElement->Attribute("numberColor");
            if (val!=nullptr)
                retVal+=QString(" number-col=\"%1\"").arg(val);
            val=rootElement->Attribute("stringColor");
            if (val!=nullptr)
                retVal+=QString(" string-col=\"%1\"").arg(val);
            val=rootElement->Attribute("characterColor");
            if (val!=nullptr)
                retVal+=QString(" character-col=\"%1\"").arg(val);
            val=rootElement->Attribute("operatorColor");
            if (val!=nullptr)
                retVal+=QString(" operator-col=\"%1\"").arg(val);
            val=rootElement->Attribute("identifierColor");
            if (val!=nullptr)
                retVal+=QString(" identifier-col=\"%1\"").arg(val);
            val=rootElement->Attribute("preprocessorColor");
            if (val!=nullptr)
                retVal+=QString(" preprocessor-col=\"%1\"").arg(val);
            val=rootElement->Attribute("wordColor");
            if (val!=nullptr)
                retVal+=QString(" keyword3-col=\"%1\"").arg(val);
            val=rootElement->Attribute("word4Color");
            if (val!=nullptr)
                retVal+=QString(" keyword4-col=\"%1\"").arg(val);
            val=rootElement->Attribute("useVrepKeywords");
            bool vrepKeywords=false;
            if (val!=nullptr)
                vrepKeywords=(strcmp(val,"true")==0);

            retVal+=" resizable=\"true\" placement=\"absolute\" font=\"Courier\" toolbar=\"false\" statusbar=\"false\"";
            retVal+=" can-restart=\"false\" max-lines=\"0\" wrap-word=\"false\" closeable=\"true\"";
            retVal+=" activate=\"false\" line-numbers=\"true\" tab-width=\"4\"";
            int fontSize=12;
            #ifdef MAC_VREP
                fontSize=16; // bigger fonts here
            #endif
            if (App::userSettings->scriptEditorFontSize!=-1)
                fontSize=App::userSettings->scriptEditorFontSize;
            #ifndef MAC_VREP
            if (App::sc>1)
                fontSize*=2;
            #endif
            retVal+=QString(" font-size=\"%1\"").arg(fontSize);

            tinyxml2::XMLElement* keywords1=rootElement->FirstChildElement("keywords1");
            if (keywords1!=nullptr)
            {
                val=keywords1->Attribute("color");
                if (val!=nullptr)
                    retVal+=QString(" keyword1-col=\"%1\"").arg(val);
            }
            tinyxml2::XMLElement* keywords2=rootElement->FirstChildElement("keywords2");
            if (keywords2!=nullptr)
            {
                val=keywords2->Attribute("color");
                if (val!=nullptr)
                    retVal+=QString(" keyword2-col=\"%1\"").arg(val);
            }
            retVal+=">";

            retVal+="<keywords1> ";
            if (keywords1!=nullptr)
            {
                tinyxml2::XMLElement* item=keywords1->FirstChildElement("item");
                while (item!=nullptr)
                {
                    retVal+="<item ";
                    val=item->Attribute("word");
                    if (val!=nullptr)
                        retVal+=QString(" word=\"%1\"").arg(val);
                    val=item->Attribute("autocomplete");
                    if (val!=nullptr)
                        retVal+=QString(" autocomplete=\"%1\"").arg(val);
                    val=item->Attribute("calltip");
                    if (val!=nullptr)
                        retVal+=QString(" calltip=\"%1\"").arg(val);
                    item=item->NextSiblingElement("item");
                    retVal+=" />";
                }
                if (vrepKeywords)
                    retVal+=getFuncKeywords(sim_scripttype_childscript,false);
            }
            retVal+="</keywords1>";

            retVal+="<keywords2> ";
            if (keywords2!=nullptr)
            {
                tinyxml2::XMLElement* item=keywords2->FirstChildElement("item");
                while (item!=nullptr)
                {
                    retVal+="<item ";
                    val=item->Attribute("word");
                    if (val!=nullptr)
                        retVal+=QString(" word=\"%1\"").arg(val);
                    val=item->Attribute("autocomplete");
                    if (val!=nullptr)
                        retVal+=QString(" autocomplete=\"%1\"").arg(val);
                    val=item->Attribute("calltip");
                    if (val!=nullptr)
                        retVal+=QString(" calltip=\"%1\"").arg(val);
                    item=item->NextSiblingElement("item");
                    retVal+=" />";
                }
                if (vrepKeywords)
                    retVal+=getVarKeywords(sim_scripttype_childscript,false);
            }
            retVal+="</keywords2>";
            retVal+="</editor>";
        }
    }
    else
        retVal+="></editor>";
    return(retVal);
}

CCodeEditorContainer::CCodeEditorContainer()
{
}

CCodeEditorContainer::~CCodeEditorContainer()
{
}

int CCodeEditorContainer::open(const char* initText,const char* xml,int callingScriptHandle)
{
    CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(callingScriptHandle);
    int retVal=-1;
    if (CPluginContainer::isCodeEditorPluginAvailable())
    {
        if (it!=NULL)
        {
            if ( (it->getScriptType()==sim_scripttype_sandboxscript)||(it->getScriptType()==sim_scripttype_addonscript) )
            {
            }
            retVal=CPluginContainer::codeEditor_open(initText,xml);
            SCodeEditor inf;
            inf.handle=retVal;
            inf.scriptHandle=-1;
            inf.callingScriptHandle=callingScriptHandle;
            inf.sceneUniqueId=App::ct->environment->getSceneUniqueID();
            inf.openAcrossScenes=( (it->getScriptType()==sim_scripttype_sandboxscript)||(it->getScriptType()==sim_scripttype_addonscript) );
            inf.closeAtSimulationEnd=it->isSimulationScript();
            inf.systemVisibility=true;
            inf.userVisibility=true;
            inf.closeAfterCallbackCalled=false;
            inf.callbackFunction="";
            _allEditors.push_back(inf);
        }
    }
    else
        printf("Code Editor plugin was not found.\n");
    return(retVal);
}

int CCodeEditorContainer::openSimulationScript(int scriptHandle,int callingScriptHandle)
{
    int sceneId=App::ct->environment->getSceneUniqueID();
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if ( (_allEditors[i].scriptHandle==scriptHandle)&&(_allEditors[i].sceneUniqueId==sceneId) )
            return(_allEditors[i].handle);
    }
    CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(scriptHandle);
    int retVal=-1;
    if (CPluginContainer::isCodeEditorPluginAvailable())
    {
        if (it!=NULL)
        {
            if (it->getScriptType()==sim_scripttype_mainscript)
            {
                if (it->isDefaultMainScript())
                { // Display warning
                    if (VMESSAGEBOX_REPLY_YES!=App::uiThread->messageBox_warning(App::mainWindow,strTranslate("Main script"),strTranslate(IDS_MAINSCRIPT_EDITION_WARNING),VMESSAGEBOX_YES_NO))
                        return(-1);
                    it->setCustomizedMainScript(true);
                    POST_SCENE_CHANGED_ANNOUNCEMENT(""); // **************** UNDO THINGY ****************
                }
            }
            int posAndSize[4];
            it->getPreviousEditionWindowPosAndSize(posAndSize);
            QString xml;
            xml+=QString("<editor title=\"%1\"").arg(it->getDescriptiveName().c_str());
            xml+=QString(" position=\"%1 %2\" size=\"%3 %4\"").arg(posAndSize[0]).arg(posAndSize[1]).arg(posAndSize[2]).arg(posAndSize[3]);
            xml+=" resizable=\"true\" closeable=\"true\" placement=\"absolute\" font=\"Courier\" toolbar=\"true\" statusbar=\"false\" wrap-word=\"false\"";
            if ( (it->getScriptType()==sim_scripttype_mainscript)||it->getThreadedExecution() )
                xml+=" can-restart=\"false\"";
            else
                xml+=" can-restart=\"true\"";
            xml+=" max-lines=\"0\" activate=\"true\" editable=\"true\" searchable=\"true\" line-numbers=\"true\" tab-width=\"4\" is-lua=\"true\"";
            int fontSize=12;
            #ifdef MAC_VREP
                fontSize=16; // bigger fonts here
            #endif
            if (App::userSettings->scriptEditorFontSize!=-1)
                fontSize=App::userSettings->scriptEditorFontSize;
            #ifndef MAC_VREP
            if (App::sc>1)
                fontSize*=2;
            #endif
            xml+=QString(" font-size=\"%1\"").arg(fontSize);
            xml+=getXmlColorString("text-col",0,0,0);
            if (it->getScriptType()==sim_scripttype_mainscript)
            {
                xml+=getXmlColorString("background-col",App::userSettings->mainScriptColor_background);
                xml+=getXmlColorString("selection-col",App::userSettings->mainScriptColor_selection);
                xml+=getXmlColorString("comment-col",App::userSettings->mainScriptColor_comment);
                xml+=getXmlColorString("number-col",App::userSettings->mainScriptColor_number);
                xml+=getXmlColorString("string-col",App::userSettings->mainScriptColor_string);
                xml+=getXmlColorString("character-col",App::userSettings->mainScriptColor_character);
                xml+=getXmlColorString("operator-col",App::userSettings->mainScriptColor_operator);
                xml+=getXmlColorString("identifier-col",App::userSettings->mainScriptColor_identifier);
                xml+=getXmlColorString("preprocessor-col",App::userSettings->mainScriptColor_preprocessor);
                xml+=getXmlColorString("keyword1-col",App::userSettings->mainScriptColor_word2);
                xml+=getXmlColorString("keyword2-col",App::userSettings->mainScriptColor_word3);
                xml+=getXmlColorString("keyword3-col",App::userSettings->mainScriptColor_word);
                xml+=getXmlColorString("keyword4-col",App::userSettings->mainScriptColor_word4);
            }
            if (it->getScriptType()==sim_scripttype_childscript)
            {
                if (it->getThreadedExecution())
                {
                    xml+=" title=\"Main script\"";
                    xml+=getXmlColorString("background-col",App::userSettings->threadedChildScriptColor_background);
                    xml+=getXmlColorString("selection-col",App::userSettings->threadedChildScriptColor_selection);
                    xml+=getXmlColorString("comment-col",App::userSettings->threadedChildScriptColor_comment);
                    xml+=getXmlColorString("number-col",App::userSettings->threadedChildScriptColor_number);
                    xml+=getXmlColorString("string-col",App::userSettings->threadedChildScriptColor_string);
                    xml+=getXmlColorString("character-col",App::userSettings->threadedChildScriptColor_character);
                    xml+=getXmlColorString("operator-col",App::userSettings->threadedChildScriptColor_operator);
                    xml+=getXmlColorString("identifier-col",App::userSettings->threadedChildScriptColor_identifier);
                    xml+=getXmlColorString("preprocessor-col",App::userSettings->threadedChildScriptColor_preprocessor);
                    xml+=getXmlColorString("keyword1-col",App::userSettings->threadedChildScriptColor_word2);
                    xml+=getXmlColorString("keyword2-col",App::userSettings->threadedChildScriptColor_word3);
                    xml+=getXmlColorString("keyword3-col",App::userSettings->threadedChildScriptColor_word);
                    xml+=getXmlColorString("keyword4-col",App::userSettings->threadedChildScriptColor_word4);
                }
                else
                {
                    xml+=getXmlColorString("background-col",App::userSettings->nonThreadedChildScriptColor_background);
                    xml+=getXmlColorString("selection-col",App::userSettings->nonThreadedChildScriptColor_selection);
                    xml+=getXmlColorString("comment-col",App::userSettings->nonThreadedChildScriptColor_comment);
                    xml+=getXmlColorString("number-col",App::userSettings->nonThreadedChildScriptColor_number);
                    xml+=getXmlColorString("string-col",App::userSettings->nonThreadedChildScriptColor_string);
                    xml+=getXmlColorString("character-col",App::userSettings->nonThreadedChildScriptColor_character);
                    xml+=getXmlColorString("operator-col",App::userSettings->nonThreadedChildScriptColor_operator);
                    xml+=getXmlColorString("identifier-col",App::userSettings->nonThreadedChildScriptColor_identifier);
                    xml+=getXmlColorString("preprocessor-col",App::userSettings->nonThreadedChildScriptColor_preprocessor);
                    xml+=getXmlColorString("keyword1-col",App::userSettings->nonThreadedChildScriptColor_word2);
                    xml+=getXmlColorString("keyword2-col",App::userSettings->nonThreadedChildScriptColor_word3);
                    xml+=getXmlColorString("keyword3-col",App::userSettings->nonThreadedChildScriptColor_word);
                    xml+=getXmlColorString("keyword4-col",App::userSettings->nonThreadedChildScriptColor_word4);
                }
            }
            xml+=">";
            xml+=getKeywords(it->getScriptType(),it->getThreadedExecution());
            xml+"</editor>";
            retVal=CPluginContainer::codeEditor_open(it->getScriptText(nullptr),xml.toStdString().c_str());
            SCodeEditor inf;
            inf.handle=retVal;
            inf.scriptHandle=scriptHandle;
            inf.callingScriptHandle=callingScriptHandle;
            inf.sceneUniqueId=App::ct->environment->getSceneUniqueID();
            inf.openAcrossScenes=false;
            inf.closeAtSimulationEnd=false;
            inf.systemVisibility=true;
            inf.userVisibility=true;
            inf.closeAfterCallbackCalled=false;
            inf.callbackFunction="";
            _allEditors.push_back(inf);
        }
    }
    else
        printf("Code Editor plugin was not found.\n");
    return(retVal);
}

int CCodeEditorContainer::openCustomizationScript(int scriptHandle,int callingScriptHandle)
{
    int sceneId=App::ct->environment->getSceneUniqueID();
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if ( (_allEditors[i].scriptHandle==scriptHandle)&&(_allEditors[i].sceneUniqueId==sceneId) )
            return(_allEditors[i].handle);
    }
    CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(scriptHandle);
    int retVal=-1;
    if (CPluginContainer::isCodeEditorPluginAvailable())
    {
        if (it!=NULL)
        {
            int posAndSize[4];
            it->getPreviousEditionWindowPosAndSize(posAndSize);
            QString xml;
            xml+=QString("<editor title=\"%1\"").arg(it->getDescriptiveName().c_str());
            xml+=QString(" position=\"%1 %2\" size=\"%3 %4\"").arg(posAndSize[0]).arg(posAndSize[1]).arg(posAndSize[2]).arg(posAndSize[3]);
            xml+=" resizable=\"true\" closeable=\"true\" placement=\"absolute\" font=\"Courier\" toolbar=\"true\" statusbar=\"false\" wrap-word=\"false\"";
            xml+=" max-lines=\"0\" can-restart=\"true\"";
            xml+=" activate=\"true\" editable=\"true\" searchable=\"true\" line-numbers=\"true\" tab-width=\"4\" is-lua=\"true\"";
            int fontSize=12;
            #ifdef MAC_VREP
                fontSize=16; // bigger fonts here
            #endif
            if (App::userSettings->scriptEditorFontSize!=-1)
                fontSize=App::userSettings->scriptEditorFontSize;
            #ifndef MAC_VREP
            if (App::sc>1)
                fontSize*=2;
            #endif
            xml+=QString(" font-size=\"%1\"").arg(fontSize);
            xml+=getXmlColorString("text-col",0,0,0);

            xml+=getXmlColorString("background-col",App::userSettings->customizationScriptColor_background);
            xml+=getXmlColorString("selection-col",App::userSettings->customizationScriptColor_selection);
            xml+=getXmlColorString("comment-col",App::userSettings->customizationScriptColor_comment);
            xml+=getXmlColorString("number-col",App::userSettings->customizationScriptColor_number);
            xml+=getXmlColorString("string-col",App::userSettings->customizationScriptColor_string);
            xml+=getXmlColorString("character-col",App::userSettings->customizationScriptColor_character);
            xml+=getXmlColorString("operator-col",App::userSettings->customizationScriptColor_operator);
            xml+=getXmlColorString("identifier-col",App::userSettings->customizationScriptColor_identifier);
            xml+=getXmlColorString("preprocessor-col",App::userSettings->customizationScriptColor_preprocessor);
            xml+=getXmlColorString("keyword1-col",App::userSettings->customizationScriptColor_word2);
            xml+=getXmlColorString("keyword2-col",App::userSettings->customizationScriptColor_word3);
            xml+=getXmlColorString("keyword3-col",App::userSettings->customizationScriptColor_word);
            xml+=getXmlColorString("keyword4-col",App::userSettings->customizationScriptColor_word4);

            xml+=">";
            xml+=getKeywords(it->getScriptType(),it->getThreadedExecution());
            xml+"</editor>";
            retVal=CPluginContainer::codeEditor_open(it->getScriptText(nullptr),xml.toStdString().c_str());
            SCodeEditor inf;
            inf.handle=retVal;
            inf.scriptHandle=scriptHandle;
            inf.callingScriptHandle=callingScriptHandle;
            inf.sceneUniqueId=App::ct->environment->getSceneUniqueID();
            inf.openAcrossScenes=false;
            inf.closeAtSimulationEnd=false;
            inf.systemVisibility=true;
            inf.userVisibility=true;
            inf.closeAfterCallbackCalled=false;
            inf.callbackFunction="";
            _allEditors.push_back(inf);
        }
    }
    else
        printf("Code Editor plugin was not found.\n");
    return(retVal);
}

int CCodeEditorContainer::openConsole(const char* title,int maxLines,int mode,const int position[2],const int size[2],const int textColor[3],const int backColor[3],int callingScriptHandle)
{
    int retVal=-1;
    if (CPluginContainer::isCodeEditorPluginAvailable())
    {
        int _position[2]={100,100};
        int _size[2]={640,200};
        int _textColor[3]={0,0,0};
        int _backColor[3]={255,255,255};
        for (size_t i=0;i<2;i++)
        {
            if (position!=nullptr)
                _position[i]=position[i];
            if (size!=nullptr)
                _size[i]=size[i];
        }
        for (size_t i=0;i<3;i++)
        {
            if (textColor!=nullptr)
                _textColor[i]=textColor[i];
            if (backColor!=nullptr)
                _backColor[i]=backColor[i];
        }
        QString xml;
        xml+=QString("<editor title=\"%1\"").arg(title);
        xml+=QString(" position=\"%1 %2\" size=\"%3 %4\"").arg(_position[0]).arg(_position[1]).arg(_size[0]).arg(_size[1]);
        xml+=" resizable=\"true\" placement=\"absolute\" font=\"Courier\" toolbar=\"false\" statusbar=\"false\"";
        xml+=" can-restart=\"false\"";
        xml+=QString(" max-lines=\"%1\"").arg(maxLines);
        xml+=" activate=\"false\" editable=\"false\" searchable=\"false\" line-numbers=\"false\" tab-width=\"4\" is-lua=\"false\"";
        if (mode&2)
            xml+="  wrap-word=\"true\"";
        else
            xml+="  wrap-word=\"false\"";
        if (mode&4)
            xml+="  closeable=\"true\"";
        else
            xml+="  closeable=\"false\"";
        int fontSize=12;
        #ifdef MAC_VREP
            fontSize=16; // bigger fonts here
        #endif
        if (App::userSettings->scriptEditorFontSize!=-1)
            fontSize=App::userSettings->scriptEditorFontSize;
        #ifndef MAC_VREP
        if (App::sc>1)
            fontSize*=2;
        #endif
        xml+=QString(" font-size=\"%1\"").arg(fontSize);
        xml+=getXmlColorString("text-col",_textColor);
        xml+=getXmlColorString("background-col",_backColor);
        xml+="></editor>";
        retVal=CPluginContainer::codeEditor_open("",xml.toStdString().c_str());
        SCodeEditor inf;
        inf.handle=retVal;
        inf.scriptHandle=-1;
        inf.callingScriptHandle=callingScriptHandle;
        inf.sceneUniqueId=App::ct->environment->getSceneUniqueID();
        inf.openAcrossScenes=((mode&16)>0);
        inf.closeAtSimulationEnd=((mode&1)>0);
        inf.systemVisibility=true;
        inf.userVisibility=true;
        inf.closeAfterCallbackCalled=false;
        inf.callbackFunction="";
        _allEditors.push_back(inf);
    }
    else
        printf("Code Editor plugin was not found.\n");
    return(retVal);
}

std::string CCodeEditorContainer::openModalTextEditor(const char* initText,const char* xml,int windowSizeAndPos[4]) const
{
    std::string retVal;
    if (CPluginContainer::isCodeEditorPluginAvailable())
    {
        QString newXml;
        if (xml!=nullptr)
            newXml=translateXml(xml,"");
        int posAndSize[4];
        CPluginContainer::codeEditor_openModal(initText,newXml.toStdString().c_str(),retVal,posAndSize);
        if (windowSizeAndPos!=nullptr)
        {
            windowSizeAndPos[0]=posAndSize[2];
            windowSizeAndPos[1]=posAndSize[3];
            windowSizeAndPos[2]=posAndSize[0];
            windowSizeAndPos[3]=posAndSize[1];
        }
    }
    else
        printf("Code Editor plugin was not found.\n");
    return(retVal);
}

int CCodeEditorContainer::openTextEditor(const char* initText,const char* xml,const char* callback,int callingScriptHandle,bool isSimulationScript)
{
    int retVal=-1;
    if (CPluginContainer::isCodeEditorPluginAvailable())
    {
        QString newXml;
        newXml=translateXml(xml,callback);
        retVal=CPluginContainer::codeEditor_open(initText,newXml.toStdString().c_str());
        SCodeEditor inf;
        inf.handle=retVal;
        inf.scriptHandle=-1;
        inf.callingScriptHandle=callingScriptHandle;
        inf.sceneUniqueId=App::ct->environment->getSceneUniqueID();
        inf.openAcrossScenes=false;
        inf.closeAtSimulationEnd=isSimulationScript;
        inf.systemVisibility=true;
        inf.userVisibility=true;
        inf.closeAfterCallbackCalled=true;
        inf.callbackFunction=callback;
        _allEditors.push_back(inf);
    }
    else
        printf("Code Editor plugin was not found.\n");
    return(retVal);
}

bool CCodeEditorContainer::close(int handle,int posAndSize[4],std::string* txt,std::string* callback)
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
        {
            if (callback!=nullptr)
                callback[0]=_allEditors[i].callbackFunction;
            std::string _txt;
            CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(_allEditors[i].scriptHandle);
            if (CPluginContainer::codeEditor_getText(handle,_txt,nullptr))
            {
                if (txt!=nullptr)
                    txt[0]=_txt;
                if (it!=nullptr)
                    it->setScriptText(_txt.c_str(),nullptr);
            }
            int pas[4];
            CPluginContainer::codeEditor_close(handle,pas);
            if (it!=nullptr)
                it->setPreviousEditionWindowPosAndSize(pas);
            if (posAndSize!=nullptr)
            {
                for (size_t j=0;j<4;j++)
                    posAndSize[j]=pas[j];
            }
            _allEditors.erase(_allEditors.begin()+i);
            return(true);
        }
    }
    return(false);
}

bool CCodeEditorContainer::closeFromScriptHandle(int scriptHandle,int posAndSize[4],bool ignoreChange)
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].scriptHandle==scriptHandle)
        {
            std::string txt;
            CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandle);
            if ((!ignoreChange)&&CPluginContainer::codeEditor_getText(_allEditors[i].handle,txt,nullptr))
            {
                if (it!=nullptr)
                    it->setScriptText(txt.c_str(),nullptr);
            }
            int pas[4];
            CPluginContainer::codeEditor_close(_allEditors[i].handle,pas);
            if (it!=nullptr)
                it->setPreviousEditionWindowPosAndSize(pas);
            if (posAndSize!=nullptr)
            {
                for (size_t j=0;j<4;j++)
                    posAndSize[j]=pas[j];
            }
            _allEditors.erase(_allEditors.begin()+i);
            return(true);
        }
    }
    return(false);
}

void CCodeEditorContainer::restartScript(int handle) const
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
        {
            std::string txt;
            CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(_allEditors[i].scriptHandle);
            if (CPluginContainer::codeEditor_getText(handle,txt,nullptr))
            {
                if ( (it!=nullptr)&&(!it->getThreadedExecution()) )
                {
                    it->setScriptText(txt.c_str(),nullptr);
                    it->killLuaState();
                }
            }
            break;
        }
    }
}

bool CCodeEditorContainer::isHandleValid(int handle) const
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
            return(true);
    }
    return(false);
}

std::string CCodeEditorContainer::getText(int handle,int posAndSize[4]) const
{
    std::string retVal;
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
        {
            CPluginContainer::codeEditor_getText(handle,retVal,posAndSize);
            break;
        }
    }
    return(retVal);
}

bool CCodeEditorContainer::setText(int handle,const char* txt) const
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
        {
            CPluginContainer::codeEditor_setText(handle,txt,0);
            return(true);
        }
    }
    return(false);
}

bool CCodeEditorContainer::appendText(int handle,const char* txt) const
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
        {
            CPluginContainer::codeEditor_setText(handle,txt,1);
            return(true);
        }
    }
    return(false);
}

int CCodeEditorContainer::getCallingScriptHandle(int handle) const
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
            return(_allEditors[i].callingScriptHandle);
    }
    return(-1);
}

bool CCodeEditorContainer::getCloseAfterCallbackCalled(int handle) const
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
            return(_allEditors[i].closeAfterCallbackCalled);
    }
    return(false);
}

void CCodeEditorContainer::simulationAboutToStart() const
{
    int sceneId=App::ct->environment->getSceneUniqueID();
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if ( (_allEditors[i].sceneUniqueId==sceneId)&&(_allEditors[i].scriptHandle>=0) )
        {
            CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(_allEditors[i].scriptHandle);
            if ( (it!=nullptr)&&((it->getScriptType()==sim_scripttype_mainscript)||(it->getScriptType()==sim_scripttype_childscript)||(it->getScriptType()==sim_scripttype_contactcallback)||(it->getScriptType()==sim_scripttype_generalcallback)||(it->getScriptType()==sim_scripttype_jointctrlcallback)) )
            {
                std::string txt;
                if (CPluginContainer::codeEditor_getText(_allEditors[i].handle,txt,nullptr))
                    it->setScriptText(txt.c_str(),nullptr);
            }
        }
    }
}

void CCodeEditorContainer::simulationAboutToEnd()
{
    int sceneId=App::ct->environment->getSceneUniqueID();
    for (int i=0;i<int(_allEditors.size());i++)
    {
        if ( (_allEditors[i].sceneUniqueId==sceneId)&&_allEditors[i].closeAtSimulationEnd )
        {
            CPluginContainer::codeEditor_close(_allEditors[i].handle,nullptr);
            _allEditors.erase(_allEditors.begin()+i);
            i--;
        }
    }
}

bool CCodeEditorContainer::areSceneEditorsOpen() const
{
    int sceneId=App::ct->environment->getSceneUniqueID();
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if ( (_allEditors[i].sceneUniqueId==sceneId)&&(_allEditors[i].scriptHandle>=0) )
            return(true);
    }
    return(false);
}

void CCodeEditorContainer::sceneClosed(int sceneUniqueId)
{
    for (int i=0;i<int(_allEditors.size());i++)
    {
        if ( (_allEditors[i].sceneUniqueId==sceneUniqueId)&&(!_allEditors[i].openAcrossScenes) )
        {
            CPluginContainer::codeEditor_close(_allEditors[i].handle,nullptr);
            _allEditors.erase(_allEditors.begin()+i);
            i--;
        }
    }
}

void CCodeEditorContainer::showOrHideAll(bool showState)
{
    if (App::ct->environment!=NULL)
    {
        int sceneId=App::ct->environment->getSceneUniqueID();
        for (size_t i=0;i<_allEditors.size();i++)
        {
            if ( (_allEditors[i].sceneUniqueId==sceneId)&&(!_allEditors[i].openAcrossScenes) )
            {
                _allEditors[i].systemVisibility=showState;
                if (showState&&_allEditors[i].userVisibility)
                    CPluginContainer::codeEditor_show(_allEditors[i].handle,1);
                else
                    CPluginContainer::codeEditor_show(_allEditors[i].handle,0);
            }
        }
    }
}

int CCodeEditorContainer::showOrHide(int handle,bool showState)
{
    int retVal=-1;
    int sceneId=App::ct->environment->getSceneUniqueID();
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
        {
            retVal=0;
            if (_allEditors[i].userVisibility!=showState)
                retVal=1;
            _allEditors[i].userVisibility=showState;
            if (showState&&_allEditors[i].systemVisibility)
                CPluginContainer::codeEditor_show(_allEditors[i].handle,1);
            else
                CPluginContainer::codeEditor_show(_allEditors[i].handle,0);
            break;
        }
    }
    return(retVal);
}

int CCodeEditorContainer::getShowState(int handle) const
{ // ret -1: no such handle, 0=user hidden, 1=user visible
    int retVal=-1;
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
        {
            retVal=0;
            if (_allEditors[i].userVisibility)
                retVal=1;
            break;
        }
    }
    return(retVal);
}
