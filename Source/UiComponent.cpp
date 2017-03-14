//
//  UiComponent.cpp
//  spatServerGRIS
//
//  Created by GRIS on 2017-03-13.
//
//

#include "UiComponent.h"
#include "LevelComponent.h"
#include "MainComponent.h"

//======================================= BOX ===========================================================================
Box::Box(GrisLookAndFeel *feel, String title) {
    
    this->title = title;
    this->grisFeel = feel;
    this->bgColour = this->grisFeel->getBackgroundColour();
    
    this->content = new Component();
    this->viewport = new Viewport();
    this->viewport->setViewedComponent(this->content, false);
    this->viewport->setScrollBarsShown(true, true);
    this->viewport->setScrollBarThickness(6);
    this->viewport->getVerticalScrollBar()->setColour(ScrollBar::ColourIds::thumbColourId, feel->getScrollBarColour());
    this->viewport->getHorizontalScrollBar()->setColour(ScrollBar::ColourIds::thumbColourId, feel->getScrollBarColour());
    
    this->viewport->setLookAndFeel(this->grisFeel);
    addAndMakeVisible(this->viewport);
}

Box::~Box(){
    delete this->viewport;
    delete this->content;
}

Component * Box::getContent() {
    return this->content ? this->content : this;
}


void Box::resized() {
    if (this->viewport){
        this->viewport->setSize(getWidth(), getHeight());
    }
}

void Box::correctSize(int width, int height){
    if(this->title!=""){
        this->viewport->setTopLeftPosition(0, 20);
        this->viewport->setSize(getWidth(), getHeight()-20);
        if(width<20){
            width = 20;
        }
    }else{
        this->viewport->setTopLeftPosition(0, 0);
    }

    this->getContent()->setSize(width, height);
}


void Box::paint(Graphics &g) {
    g.setColour(this->bgColour);
    g.fillRect(getLocalBounds());
    if(this->title!=""){
        g.setColour (this->grisFeel->getWinBackgroundColour());
        g.fillRect(0,0,getWidth(),18);
        
        g.setColour (this->grisFeel->getFontColour());
        g.drawText(title, 0, 0, this->content->getWidth(), 20, juce::Justification::left);
    }
}




//======================================= LevelBox =====================================================================
LevelBox::LevelBox(LevelComponent * parent, GrisLookAndFeel *feel):
    mainParent(parent),
    grisFeel(feel)
{

}

LevelBox::~LevelBox(){
    
}


void LevelBox::setBounds(const Rectangle<int> &newBounds){
    this->juce::Component::setBounds(newBounds);
    colorGrad = ColourGradient(Colours::red, 0.f, 0.f, Colour::fromRGB(17, 255, 159), 0.f, getHeight(), false);
    colorGrad.addColour(0.1, Colours::yellow);
}

void LevelBox::paint (Graphics& g){
    if(this->mainParent->isMuted()){
        g.fillAll (grisFeel->getWinBackgroundColour());
    }
    else{
        float level = this->mainParent->getLevel();
        g.setGradientFill(colorGrad);
        g.fillRect(0, 0, getWidth() ,getHeight());
        
        if (level < MinLevelComp){
            level = MinLevelComp;
        }
        if (level < 0.9f){
            level = -abs(level);
            g.setColour(grisFeel->getDarkColour());
            g.fillRect(0, 0, getWidth() ,(int)(getHeight()*(level/MinLevelComp)));
        }
    }
}


//======================================= Window Edit Speaker============================================================

WindowEditSpeaker::WindowEditSpeaker(const String& name, Colour backgroundColour, int buttonsNeeded,
                                     MainContentComponent * parent, GrisLookAndFeel * feel):
    DocumentWindow (name, backgroundColour, buttonsNeeded), font (14.0f)
{
    this->mainParent = parent;
    this->grisFeel = feel;
    
}
WindowEditSpeaker::~WindowEditSpeaker(){
    //delete this->labColumn;
    //delete this->boxListSpeaker;
    this->mainParent->destroyWinSpeakConf();
}
void WindowEditSpeaker::initComp(){
    this->boxListSpeaker = new Box(this->grisFeel, "Configuration Speakers");
    this->setContentComponent(this->boxListSpeaker);
    
    
    /*this->labColumn = new Label();
    this->labColumn->setText("X                   Y                   Z                       Azimuth         Zenith          Radius          #",
                             NotificationType::dontSendNotification);
    this->labColumn->setJustificationType(Justification::left);
    this->labColumn->setFont(this->grisFeel->getFont());
    this->labColumn->setLookAndFeel(this->grisFeel);
    this->labColumn->setColour(Label::textColourId, this->grisFeel->getFontColour());
    this->labColumn->setBounds(25, 0, getWidth(), 22);*/
    this->boxListSpeaker->getContent()->addAndMakeVisible(tableListSpeakers);
    tableListSpeakers.setModel(this);
    
    tableListSpeakers.setColour (ListBox::outlineColourId, this->grisFeel->getWinBackgroundColour());
    tableListSpeakers.setColour(ListBox::backgroundColourId, this->grisFeel->getWinBackgroundColour());
    tableListSpeakers.setOutlineThickness (1);
    
    tableListSpeakers.getHeader().addColumn("ID", 1, 40, 40, 60,TableHeaderComponent::defaultFlags);
    
    tableListSpeakers.getHeader().addColumn("X", 2, 70, 50, 120,TableHeaderComponent::defaultFlags);
    tableListSpeakers.getHeader().addColumn("Y", 3, 70, 50, 120,TableHeaderComponent::defaultFlags);
    tableListSpeakers.getHeader().addColumn("Z", 4, 70, 50, 120,TableHeaderComponent::defaultFlags);
    
    tableListSpeakers.getHeader().addColumn("Azimuth", 5, 70, 50, 120,TableHeaderComponent::defaultFlags);
    tableListSpeakers.getHeader().addColumn("Zenith", 6, 70, 50, 120,TableHeaderComponent::defaultFlags);
    tableListSpeakers.getHeader().addColumn("Radius", 7, 70, 50, 120,TableHeaderComponent::defaultFlags);
    
    tableListSpeakers.getHeader().addColumn("Output", 8, 40, 40, 60,TableHeaderComponent::defaultFlags);
    
    tableListSpeakers.getHeader().setSortColumnId (1, true); // sort forwards by the ID column
    //tableListSpeakers.getHeader().setColumnVisible (7, false);
    
    tableListSpeakers.setMultipleSelectionEnabled (false);
    
    numRows =this->mainParent->getListSpeaker().size();


    
    this->boxListSpeaker->setBounds(0, 0, getWidth(),getHeight());
    this->boxListSpeaker->correctSize(getWidth()-8, (this->mainParent->getListSpeaker().size()*28)+50);
    tableListSpeakers.setSize(getWidth(), getHeight());
    
    tableListSpeakers.updateContent();
    tableListSpeakers.repaint();
}

void WindowEditSpeaker::closeButtonPressed()
{
    delete this;
}



String WindowEditSpeaker::getText (const int columnNumber, const int rowNumber) const
{
    String text = "";
    this->mainParent->getLockSpeakers()->lock();
    if (this->mainParent->getListSpeaker().size()> rowNumber)
    {
        
        switch(columnNumber){
            case 1 :
                text =String(this->mainParent->getListSpeaker()[rowNumber]->getIdSpeaker());
                break;
                
            case 2 :
                text =String(this->mainParent->getListSpeaker()[rowNumber]->getCoordinate().x);
                break;
            case 3 :
                text =String(this->mainParent->getListSpeaker()[rowNumber]->getCoordinate().y);
                break;
            case 4 :
                text =String(this->mainParent->getListSpeaker()[rowNumber]->getCoordinate().z);
                break;
                
            case 5 :
                text =String(this->mainParent->getListSpeaker()[rowNumber]->getAziZenRad().x);
                break;
            case 6 :
                text =String(this->mainParent->getListSpeaker()[rowNumber]->getAziZenRad().y);
                break;
            case 7 :
                text =String(this->mainParent->getListSpeaker()[rowNumber]->getAziZenRad().z);
                break;
                
            case 8 :
                text =String(this->mainParent->getListSpeaker()[rowNumber]->getOutputPatch());
                break;
                
            default:
                text ="?";
        }
    }

    this->mainParent->getLockSpeakers()->unlock();
    return text;
}

void WindowEditSpeaker::setText (const int columnNumber, const int rowNumber, const String& newText)
{
    this->mainParent->getLockSpeakers()->lock();
    if (this->mainParent->getListSpeaker().size()> rowNumber)
    {
        glm::vec3 newP;
        switch(columnNumber){
                
            case 2 :
                newP = this->mainParent->getListSpeaker()[rowNumber]->getCoordinate();
                newP.x = newText.getFloatValue();
                this->mainParent->getListSpeaker()[rowNumber]->setCoordinate(newP);

                break;
            case 3 :
                newP = this->mainParent->getListSpeaker()[rowNumber]->getCoordinate();
                newP.y = newText.getFloatValue();
                this->mainParent->getListSpeaker()[rowNumber]->setCoordinate(newP);
                break;
            case 4 :
                newP = this->mainParent->getListSpeaker()[rowNumber]->getCoordinate();
                newP.z = newText.getFloatValue();
                this->mainParent->getListSpeaker()[rowNumber]->setCoordinate(newP);
                break;
                
            case 5 :
                newP = this->mainParent->getListSpeaker()[rowNumber]->getAziZenRad();
                newP.x = newText.getFloatValue();
                this->mainParent->getListSpeaker()[rowNumber]->setAziZenRad(newP);
                break;
            case 6 :
                newP = this->mainParent->getListSpeaker()[rowNumber]->getAziZenRad();
                newP.y = newText.getFloatValue();
                this->mainParent->getListSpeaker()[rowNumber]->setAziZenRad(newP);
                break;
            case 7 :
                newP = this->mainParent->getListSpeaker()[rowNumber]->getAziZenRad();
                newP.z = newText.getFloatValue();
                this->mainParent->getListSpeaker()[rowNumber]->setAziZenRad(newP);
                break;
                
            case 8 :
                this->mainParent->getListSpeaker()[rowNumber]->setOutputPatch(newText.getIntValue());
                break;
                
        }
    }
    
    this->mainParent->getLockSpeakers()->unlock();
}

int WindowEditSpeaker::getNumRows()
{
    return numRows;
}

// This is overloaded from TableListBoxModel, and should fill in the background of the whole row
void WindowEditSpeaker::paintRowBackground (Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected)
{
    if (rowIsSelected){
        this->mainParent->getLockSpeakers()->lock();
        this->mainParent->getListSpeaker()[rowNumber]->selectSpeaker();
        this->mainParent->getLockSpeakers()->unlock();
        g.fillAll (this->grisFeel->getOnColour());
    }
    else{
        this->mainParent->getLockSpeakers()->lock();
        this->mainParent->getListSpeaker()[rowNumber]->unSelectSpeaker();
        this->mainParent->getLockSpeakers()->unlock();
        if (rowNumber % 2){
            g.fillAll (this->grisFeel->getBackgroundColour().withBrightness(0.6));
        }else{
            g.fillAll (this->grisFeel->getBackgroundColour().withBrightness(0.7));
        }
    }
}

// This is overloaded from TableListBoxModel, and must paint any cells that aren't using custom
// components.
void WindowEditSpeaker::paintCell (Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/)
{
    g.setColour (Colours::black);
    g.setFont (font);
    
    this->mainParent->getLockSpeakers()->lock();
    if (this->mainParent->getListSpeaker().size()> rowNumber)
    {
        String text = getText(columnId, rowNumber);
        g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
        
    }
    this->mainParent->getLockSpeakers()->unlock();
    g.setColour (Colours::black.withAlpha (0.2f));
    g.fillRect (width - 1, 0, 1, height);
}

Component* WindowEditSpeaker::refreshComponentForCell (int rowNumber, int columnId, bool /*isRowSelected*/,
                                    Component* existingComponentToUpdate)
{
    
    // The other columns are editable text columns, for which we use the custom Label component
    EditableTextCustomComponent* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);
    
    // same as above...
    if (textLabel == nullptr)
        textLabel = new EditableTextCustomComponent (*this);
    
    textLabel->setRowAndColumn (rowNumber, columnId);
    if(columnId==1){    //ID Speakers
        textLabel->setEditable(false);
    }
    return textLabel;
}

