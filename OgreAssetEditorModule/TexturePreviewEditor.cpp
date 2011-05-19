/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   TexturePreviewEditor.cpp
 *  @brief  Preview window for textures.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "TexturePreviewEditor.h"
#include "OgreAssetEditorModule.h"

#include "Application.h"
#include "LoggingFunctions.h"

#include <QUiLoader>
#include <QFile>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QLayout>
#include <QScrollArea>

#include "MemoryLeakCheck.h"

TextureLabel::TextureLabel(QWidget *parent, Qt::WindowFlags flags):
    QLabel(parent, flags)
{
}

TextureLabel::~TextureLabel()
{
}

void TextureLabel::mousePressEvent(QMouseEvent *ev)
{
    emit MouseClicked(ev);
}

TexturePreviewEditor::~TexturePreviewEditor()
{
    LogInfo("Deleting TexturePreviewEditor " + objectName());
}

TexturePreviewEditor::TexturePreviewEditor(QWidget* parent) :
    QWidget(parent),
    mainWidget_(0),
    okButtonName_(0),
    headerLabel_(0),
    imageLabel_(0),
    scaleLabel_(0),
    layout_(0),
    imageSize_(QSize(0,0)),
    useOriginalImageSize_(true)
{
     Initialize();
}

void TexturePreviewEditor::Close()
{
//    UiServiceInterface* ui= framework_->G et Service<UiServiceInterface>();
//    if (!ui)
//        return
//    ui->RemoveWidgetFromScene(this);
    // Must be last line in this function, since it is possible this causes the deletion of this object
    emit Closed("");
}

void TexturePreviewEditor::RequestTextureAsset(const QString &asset_id)
{
    ///\todo Regression. Reimplement using the new Asset API. -jj.
/*
    ServiceManagerPtr service_manager = framework_->GetSer vi ceManager();
    if(service_manager)
    {
        if(service_manager->IsRegistered(Service::ST_Texture))
        {
            boost::shared_ptr<TextureServiceInterface> texture_service = 
                service_manager->G et Service<TextureServiceInterface>(Service::ST_Texture).lock();
            if(!texture_service)
                return;
            // Request texture assets.
            request_tag_ = texture_service->RequestTexture(asset_id.toStdString());
        }
    }
*/
}
/*
    ///\todo Regression. Reimplement using the new Asset API. -jj.
void TexturePreviewEditor::HandleResouceReady(Resource::Events::ResourceReady *res)
{
    if(request_tag_ == res->tag_)
    {
        TextureInterface *tex = dynamic_cast<TextureInterface *>(res->resource_.get());
        if(tex)
        {
            QImage img = ConvertToQImage(tex->GetData(), tex->GetWidth(), tex->GetHeight(), tex->GetComponents());
            // Only show chessboard pattern if image has an alpha channel.
            if(tex->GetComponents() == 4 || tex->GetComponents() == 2) 
            {
                imageLabel_->setStyleSheet("QLabel#previewImageLabel"
                                           "{"
                                                "background-image: url(\Application::InstallationDirectory + "data/ui/images/image_background.png\");"
                                                "background-repeat: repeat-xy;"
                                           "}");
            }

            if(!img.isNull())
            {
                // Take image's original size.
                imageSize_ = QSize(tex->GetWidth(), tex->GetHeight());
                if(headerLabel_)
                    headerLabel_->setText(headerLabel_->text() + QString(" %1 x %2").arg(tex->GetWidth()).arg(tex->GetHeight()));

                if(imageLabel_)
                {
                    imageLabel_->setPixmap(QPixmap::fromImage(img));
                    UseTextureOriginalSize(true);
                    scaleLabel_->setText("Image scale: 1:1");
                }
            }
        }
    }
}
*/

void TexturePreviewEditor::TextureLabelClicked(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton)
    {
        if(imageLabel_)
        {
            if(useOriginalImageSize_)
            {
                UseTextureOriginalSize(false);
            }
            else
            {
                UseTextureOriginalSize(true);
                scaleLabel_->setText("Image scale: 1:1");
            }
        }
        updateGeometry();
    }
}

void TexturePreviewEditor::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
    if(!useOriginalImageSize_ && scrollAreaWidget_)
    {
        if(scrollAreaWidget_->widget())
        {
            // Scale image but keep it's aspect ratio same.
            float pixmapAspectRatio[2];
            pixmapAspectRatio[0] = float(imageSize_.width()) / float(imageSize_.height());
            pixmapAspectRatio[1] = float(imageSize_.height()) / float(imageSize_.width());
            QSize newImageSize;
            // TODO Brutal force approach might be too heavy.
            for(uint i = 0; i < 2; i++)
            {
                if(i == 0)
                    newImageSize = QSize((scrollAreaWidget_->size().height() - 4) * pixmapAspectRatio[i], (scrollAreaWidget_->size().height() - 4));
                else
                    newImageSize = QSize((scrollAreaWidget_->size().width() - 4), (scrollAreaWidget_->size().width() - 4) * pixmapAspectRatio[i]);
                //Check if image fits into scroll area and if it does no need to continue.
                if(newImageSize.width() <= scrollAreaWidget_->size().width() && newImageSize.height() <= scrollAreaWidget_->size().height())
                {
                    scrollAreaWidget_->widget()->resize(newImageSize);
                    break;
                }
            }
        }
        float scale = CalculateImageScale();
        if(scale > 0)
            scaleLabel_->setText(QString("Image scale: %1%").arg(scale));
    }
}

void TexturePreviewEditor::Initialize()
{
    // Create widget from ui file
    QUiLoader loader;
    QFile file(Application::InstallationDirectory() + "data/ui/texture_preview.ui");
    if (!file.exists())
    {
        LogError("Cannot find OGRE Script Editor .ui file.");
        return;
    }

    mainWidget_ = loader.load(&file);
    file.close();

    setAttribute(Qt::WA_DeleteOnClose);

    resize(cWindowMinimumWidth, cWindowMinimumHeight);

    layout_ = new QVBoxLayout;
    layout_->addWidget(mainWidget_);
    layout_->setContentsMargins(0, 0, 0, 0);
    setLayout(layout_);

    // Get controls
    okButtonName_ = mainWidget_->findChild<QPushButton *>("okButton");
    connect(okButtonName_, SIGNAL(clicked()), this, SLOT(Closed()));

    headerLabel_ = mainWidget_->findChild<QLabel *>("imageNameLabel");
    scaleLabel_ = mainWidget_->findChild<QLabel *>("imageScaleLabel");

    QLabel *assetIdLabel = mainWidget_->findChild<QLabel *>("imageAssetIdLabel");
    if (assetIdLabel)
        assetIdLabel->setText("inventoryId_");

    imageLabel_ = new TextureLabel();
    imageLabel_->setObjectName("previewImageLabel");
    imageLabel_->setScaledContents(true);
    imageLabel_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    QObject::connect(imageLabel_, SIGNAL(MouseClicked(QMouseEvent*)), this, SLOT(TextureLabelClicked(QMouseEvent*)));

    scrollAreaWidget_ = mainWidget_->findChild<QScrollArea *>("imageScrollArea");
    scrollAreaWidget_->widget()->layout()->addWidget(imageLabel_);

    // Set black background image that will be replaced once the real image has been received.
    QImage emptyImage = QImage(QSize(256, 256), QImage::Format_ARGB32);
    emptyImage.fill(qRgba(0,0,0,0));
    imageLabel_->setPixmap(QPixmap::fromImage(emptyImage));
    headerLabel_->setText(objectName());

    // Add widget to UI via ui services module
    setWindowTitle(tr("Texture: ") + objectName());
//    UiProxyWidget *proxy = ui->AddWidgetToScene(this);
//    connect(proxy, SIGNAL(Closed()), this, SLOT(Closed()));
//    proxy->show();
//    ui->BringWidgetToFront(proxy);
}

void TexturePreviewEditor::OpenOgreTexture(const QString& name)
{
    if (name.contains(".dds"))
    {
        LogWarning("currently cannot show .dds files. ");
        // Set black background image that will be replaced once the real image has been received.
        QImage emptyImage = QImage(QSize(256, 256), QImage::Format_ARGB32);
        emptyImage.fill(qRgba(0,0,0,0));
        if ( imageLabel_ != 0)
            imageLabel_->setPixmap(QPixmap::fromImage(emptyImage));

        return;
    }

    Ogre::ResourcePtr res = Ogre::TextureManager::getSingleton().getByName(name.toStdString().c_str());
    Ogre::Texture* tex = static_cast<Ogre::Texture* >(res.get());
    if (!tex)
    {
        LogWarning("Failed to open Ogre texture " + name.toStdString() + " .");
        return;
    }

    int width = tex->getWidth();
    int height = tex->getHeight();
    Ogre::Box bounds(0, 0, width, height);
    Ogre::uchar* pixelData = new Ogre::uchar[width * height * 4];
    Ogre::PixelBox pixels(bounds, Ogre::PF_A8R8G8B8, pixelData);
    tex->getBuffer()->blitToMemory(pixels);
    
    // Create image of texture, and show it into label.

    u8* p = static_cast<u8 *>(pixels.data);
    int widthPixels = pixels.getWidth();
    int heightPixels= pixels.getHeight();
    
    QImage img = ConvertToQImage(p, widthPixels, heightPixels, 4);

    if(!img.isNull() && imageLabel_ != 0)
    {
        imageLabel_->setPixmap(QPixmap::fromImage(img));
        imageLabel_->show();
    }

    delete[] pixelData;
}

TexturePreviewEditor *TexturePreviewEditor::OpenPreviewEditor(const QString &texture, QWidget* parent)
{
    TexturePreviewEditor *editor = new TexturePreviewEditor(parent);
    //connect(editor, SIGNAL(Closed(const QString &)), editor, SLOT(Deleted()), Qt::QueuedConnection);
    connect(editor, SIGNAL(Closed(const QString &)), editor, SLOT(close()));
    editor->OpenOgreTexture(texture);
    return editor;
    //editor->show();
}

void TexturePreviewEditor::UseTextureOriginalSize(bool use)
{
    if(use) //Set texture to it's real size.
    {
        scrollAreaWidget_->setWidgetResizable(false);
        scrollAreaWidget_->widget()->resize(imageSize_);
        scrollAreaWidget_->widget()->setMinimumSize(imageSize_);
        scrollAreaWidget_->widget()->setMaximumSize(imageSize_);
        //Do we want to display the texture in it's real size or not. if texture's size is over 512 x 512 we will stay 
        //at smaller window size cause that texture would take too much screen space.
        if(imageSize_.width() <= 512 && imageSize_.height() <= 512)
        {
            QSize mainWidgetSize = QSize(imageSize_.width() + 12, imageSize_.height() + 80);
            if(mainWidgetSize.width() > cWindowMinimumWidth)
                mainWidgetSize.setWidth(mainWidgetSize.width());
            else
                mainWidgetSize.setWidth(cWindowMinimumWidth);

            if(mainWidgetSize.height() > cWindowMinimumHeight)
                mainWidgetSize.setHeight(mainWidgetSize.height());
            else
                mainWidgetSize.setHeight(cWindowMinimumHeight);

            resize(mainWidgetSize);
        }
        useOriginalImageSize_ = true;
    }
    else //scale texture based on it's parent widget size.
    {
        scrollAreaWidget_->setWidgetResizable(false);
        scrollAreaWidget_->widget()->setMaximumSize(16777215, 16777215);
        scrollAreaWidget_->widget()->setMinimumSize(0, 0);
        scrollAreaWidget_->widget()->resize(imageSize_);
        useOriginalImageSize_ = false;
    }

    float scale = CalculateImageScale();
    if(scale > 0)
        scaleLabel_->setText(QString("Image scale: %1%").arg(scale));
}

float TexturePreviewEditor::CalculateImageScale()
{
    if(imageLabel_)
    {
        long newImageSize = imageLabel_->size().width() * imageLabel_->size().height();
        long originalImageSize = imageSize_.width() * imageSize_.height();
        if(newImageSize > 0 && originalImageSize > 0)
            return (float(newImageSize) / float(originalImageSize)) * 100;
    }
    return -1.0f;
}

QImage TexturePreviewEditor::ConvertToQImage(const u8 *raw_image_data, uint width, uint height, uint channels)
{
    uint img_width_step = width * channels; 
    QImage image;
    if(width <= 0 && height <= 0 && channels <= 0)
        return image;

    if(channels == 3)// For RGB32
    {
        image = QImage(QSize(width, height), QImage::Format_RGB888);
        for(uint h = 0; h < height; h++)
        {
            for(uint w = 0; w < width; w++)
            {
                u8 color[3];
                for(uint comp = 0; comp < 3; comp++)
                {
                    uint index = (h % height) * (img_width_step) + ((w * channels) % (img_width_step)) + comp;
                    color[comp] = raw_image_data[index];
                }
                image.setPixel(w, h, qRgb(color[0], color[1], color[2]));
            }
        }
    }
    else if(channels == 4)// For ARGB32
    {
        image = QImage(QSize(width, height), QImage::Format_ARGB32);
        for(uint h = 0; h < height; h++)
        {
            for(uint w = 0; w < width; w++)
            {
                u8 color[4];
                for(uint comp = 0; comp < 4; comp++)
                {
                    uint index = (h % height) * (img_width_step) + ((w * channels) % (img_width_step)) + comp;
                    color[comp] = raw_image_data[index];
                }
                image.setPixel(w, h, qRgba(color[0], color[1], color[2], color[3]));
            }
        }
    }
    else if(channels == 1)// For GrayScale
    {
        image = QImage(QSize(width, height), QImage::Format_RGB888);
        for(uint h = 0; h < height; h++)
        {
            for(uint w = 0; w < width; w++)
            {
                u8 color[1];
                for(uint comp = 0; comp < 1; comp++)
                {
                    uint index = (h % height) * (img_width_step) + ((w * channels) % (img_width_step)) + comp;
                    color[comp] = raw_image_data[index];
                }
                image.setPixel(w, h, qRgb(color[0], color[0], color[0]));
            }
        }
    }
    else if(channels == 2) // contains Grayscale and Alfa channels.
    {
        // TODO! 2 channel image contain grayscale and alfa channels this code wasn't tested and might not work as planned.
        // This image format is not so much used so it might be a good idea to remove this if necessary.
        image = QImage(QSize(width, height), QImage::Format_ARGB32);
        for(uint h = 0; h < height; h++)
        {
            for(uint w = 0; w < width; w++)
            {
                u8 color[2];
                for(uint comp = 0; comp < 2; comp++)
                {
                    uint index = (h % height) * (img_width_step) + ((w * channels) % (img_width_step)) + comp;
                    color[comp] = raw_image_data[index];
                }
                image.setPixel(w, h, qRgba(color[0], color[0], color[0], color[1]));
            }
        }
    }

    return image;
}
