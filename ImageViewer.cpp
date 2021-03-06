#include "ImageViewer.h"

ImageViewer::ImageViewer(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass)
{
	ui->setupUi(this);
}

//ViewerWidget functions
ViewerWidget* ImageViewer::getViewerWidget(int tabId)
{
	QScrollArea* s = static_cast<QScrollArea*>(ui->tabWidget->widget(tabId));
	if (s) {
		ViewerWidget* vW = static_cast<ViewerWidget*>(s->widget());
		return vW;
	}
	return nullptr;
}
ViewerWidget* ImageViewer::getCurrentViewerWidget()
{
	return getViewerWidget(ui->tabWidget->currentIndex());
}

// Event filters
bool ImageViewer::eventFilter(QObject* obj, QEvent* event)
{
	if (obj->objectName() == "ViewerWidget") {
		return ViewerWidgetEventFilter(obj, event);
	}
	return false;
}

//ViewerWidget Events
bool ImageViewer::ViewerWidgetEventFilter(QObject* obj, QEvent* event)
{
	ViewerWidget* w = static_cast<ViewerWidget*>(obj);

	if (!w) {
		return false;
	}

	if (event->type() == QEvent::MouseButtonPress) {
		ViewerWidgetMouseButtonPress(w, event);
	}
	else if (event->type() == QEvent::MouseButtonRelease) {
		ViewerWidgetMouseButtonRelease(w, event);
	}
	else if (event->type() == QEvent::MouseMove) {
		ViewerWidgetMouseMove(w, event);
	}
	else if (event->type() == QEvent::Leave) {
		ViewerWidgetLeave(w, event);
	}
	else if (event->type() == QEvent::Enter) {
		ViewerWidgetEnter(w, event);
	}
	else if (event->type() == QEvent::Wheel) {
		ViewerWidgetWheel(w, event);
	}

	return QObject::eventFilter(obj, event);
}
void ImageViewer::ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	if (e->button() == Qt::LeftButton) {
		w->setFreeDrawBegin(e->pos());
		w->setFreeDrawActivated(true);
	}
}
void ImageViewer::ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	if (e->button() == Qt::LeftButton && w->getFreeDrawActivated()) {
		w->freeDraw(e->pos(), QPen(Qt::red));
		w->setFreeDrawActivated(false);
	}
}
void ImageViewer::ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	if (e->buttons() == Qt::LeftButton && w->getFreeDrawActivated()) {
		w->freeDraw(e->pos(), QPen(Qt::red));
		//w->freeDrawDDA(e->pos(), Qt::red);
		w->setFreeDrawBegin(e->pos());
	}
}
void ImageViewer::ViewerWidgetLeave(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetEnter(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetWheel(ViewerWidget* w, QEvent* event)
{
	QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
}

//ImageViewer Events
void ImageViewer::closeEvent(QCloseEvent* event)
{
	if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
	{
		event->accept();
	}
	else {
		event->ignore();
	}
}

//Image functions
void ImageViewer::openNewTabForImg(ViewerWidget* vW)
{
	QScrollArea* scrollArea = new QScrollArea;
	scrollArea->setWidget(vW);

	scrollArea->setBackgroundRole(QPalette::Dark);
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);

	QString name = vW->getName();

	ui->tabWidget->addTab(scrollArea, name);
}
bool ImageViewer::openImage(QString filename)
{
	QFileInfo fi(filename);

	QString name = fi.baseName();
	openNewTabForImg(new ViewerWidget(name, QSize(0, 0)));
	ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);

	ViewerWidget* w = getCurrentViewerWidget();

	QImage loadedImg(filename);
	return w->setImage(loadedImg);
}
bool ImageViewer::saveImage(QString filename)
{
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();
	ViewerWidget* w = getCurrentViewerWidget();

	QImage* img = w->getImage();
	return img->save(filename, extension.toStdString().c_str());
}
void ImageViewer::clearImage()
{
	ViewerWidget* w = getCurrentViewerWidget();
	w->clear();
}

//Slots

//Tabs slots
void ImageViewer::on_tabWidget_tabCloseRequested(int tabId)
{
	ViewerWidget* vW = getViewerWidget(tabId);
	delete vW; //vW->~ViewerWidget();
	ui->tabWidget->removeTab(tabId);
}
void ImageViewer::on_actionRename_triggered()
{
	if (!isImgOpened()) {
		msgBox.setText("No image is opened.");
		msgBox.setIcon(QMessageBox::Information);
		msgBox.exec();
		return;
	}
	ViewerWidget* w = getCurrentViewerWidget();
	bool ok;
	QString text = QInputDialog::getText(this, QString("Rename image"), tr("Image name:"), QLineEdit::Normal, w->getName(), &ok);
	if (ok && !text.trimmed().isEmpty())
	{
		w->setName(text);
		ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), text);
	}
}

//Image slots
void ImageViewer::on_actionNew_triggered()
{
	newImgDialog = new NewImageDialog(this);
	connect(newImgDialog, SIGNAL(accepted()), this, SLOT(newImageAccepted()));
	newImgDialog->exec();
}
void ImageViewer::newImageAccepted()
{
	NewImageDialog* newImgDialog = static_cast<NewImageDialog*>(sender());

	int width = newImgDialog->getWidth();
	int height = newImgDialog->getHeight();
	QString name = newImgDialog->getName();
	openNewTabForImg(new ViewerWidget(name, QSize(width, height)));
	ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
}
void ImageViewer::on_actionOpen_triggered()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load image", folder, fileFilter);
	if (fileName.isEmpty()) { return; }

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());

	if (!openImage(fileName)) {
		msgBox.setText("Unable to open image.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
}
void ImageViewer::on_actionSave_as_triggered()
{
	if (!isImgOpened()) {
		msgBox.setText("No image to save.");
		msgBox.setIcon(QMessageBox::Information);
		msgBox.exec();
		return;
	}
	QString folder = settings.value("folder_img_save_path", "").toString();

	ViewerWidget* w = getCurrentViewerWidget();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", folder + "/" + w->getName(), fileFilter);
	if (fileName.isEmpty()) { return; }

	QFileInfo fi(fileName);
	settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

	if (!saveImage(fileName)) {
		msgBox.setText("Unable to save image.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
	else {
		msgBox.setText(QString("File %1 saved.").arg(fileName));
		msgBox.setIcon(QMessageBox::Information);
		msgBox.exec();
	}
}
void ImageViewer::on_actionClear_triggered()
{
	if (!isImgOpened()) {
		msgBox.setText("No image is opened.");
		msgBox.setIcon(QMessageBox::Information);
		msgBox.exec();
		return;
	}
	clearImage();
}

void ImageViewer::on_generuj_clicked() {

	//pridaj vycistenie octa
	if (!octa.HisEmpty())
		octa.clear();

	int i;
	QList<Vertex>* Vrcholy;
	Vrcholy = new QList<Vertex>;
	Vrcholy->reserve(6);
	for (i = 0; i < 6; i++) {
		*Vrcholy << Vertex();
	}

	QList<H_Edge>* Polohrany;
	Polohrany = new QList<H_Edge>;
	Polohrany->reserve(24);
	for (i = 0; i < 24; i++) {
		*Polohrany << H_Edge();
	}

	QList<Face>* Steny;
	Steny = new QList<Face>;
	Steny->reserve(8);
	for (i = 0; i < 8; i++) {
		*Steny << Face();
	}

	(*Vrcholy)[0].set(0, 0.0, 0.0, 1.0);
	(*Vrcholy)[1].set(1, -1.0, 0.0, 0.0);
	(*Vrcholy)[2].set(2, 0.0, -1.0, 0.0);
	(*Vrcholy)[3].set(3, 1.0, 0.0, 0.0);
	(*Vrcholy)[4].set(4, 0.0, 1.0, 0.0);
	(*Vrcholy)[5].set(5, 0.0, 0.0, -1.0);
	

	(*Polohrany)[0].set(&(*Vrcholy)[1], &(*Steny)[0], &(*Polohrany)[2], &(*Polohrany)[1], &(*Polohrany)[12]);
	(*Polohrany)[1].set(&(*Vrcholy)[2], &(*Steny)[0], &(*Polohrany)[0], &(*Polohrany)[2], &(*Polohrany)[5]);
	(*Polohrany)[2].set(&(*Vrcholy)[0], &(*Steny)[0], &(*Polohrany)[1], &(*Polohrany)[0], &(*Polohrany)[10]);

	(*Polohrany)[3].set(&(*Vrcholy)[2], &(*Steny)[1], &(*Polohrany)[5], &(*Polohrany)[4], &(*Polohrany)[15]);
	(*Polohrany)[4].set(&(*Vrcholy)[3], &(*Steny)[1], &(*Polohrany)[3], &(*Polohrany)[5], &(*Polohrany)[8]);
	(*Polohrany)[5].set(&(*Vrcholy)[0], &(*Steny)[1], &(*Polohrany)[4], &(*Polohrany)[3], &(*Polohrany)[1]);

	(*Polohrany)[6].set(&(*Vrcholy)[3], &(*Steny)[2], &(*Polohrany)[8], &(*Polohrany)[7], &(*Polohrany)[18]);
	(*Polohrany)[7].set(&(*Vrcholy)[4], &(*Steny)[2], &(*Polohrany)[6], &(*Polohrany)[8], &(*Polohrany)[11]);
	(*Polohrany)[8].set(&(*Vrcholy)[0], &(*Steny)[2], &(*Polohrany)[7], &(*Polohrany)[6], &(*Polohrany)[4]);

	(*Polohrany)[9].set(&(*Vrcholy)[4], &(*Steny)[3], &(*Polohrany)[11], &(*Polohrany)[10], &(*Polohrany)[21]);
	(*Polohrany)[10].set(&(*Vrcholy)[1], &(*Steny)[3], &(*Polohrany)[9], &(*Polohrany)[11], &(*Polohrany)[2]);
	(*Polohrany)[11].set(&(*Vrcholy)[0], &(*Steny)[3], &(*Polohrany)[10], &(*Polohrany)[9], &(*Polohrany)[7]);

	(*Polohrany)[12].set(&(*Vrcholy)[2], &(*Steny)[4], &(*Polohrany)[14], &(*Polohrany)[13], &(*Polohrany)[0]);
	(*Polohrany)[13].set(&(*Vrcholy)[1], &(*Steny)[4], &(*Polohrany)[12], &(*Polohrany)[14], &(*Polohrany)[23]);
	(*Polohrany)[14].set(&(*Vrcholy)[5], &(*Steny)[4], &(*Polohrany)[13], &(*Polohrany)[12], &(*Polohrany)[16]);

	(*Polohrany)[15].set(&(*Vrcholy)[3], &(*Steny)[5], &(*Polohrany)[17], &(*Polohrany)[16], &(*Polohrany)[3]);
	(*Polohrany)[16].set(&(*Vrcholy)[2], &(*Steny)[5], &(*Polohrany)[15], &(*Polohrany)[17], &(*Polohrany)[16]);
	(*Polohrany)[17].set(&(*Vrcholy)[5], &(*Steny)[5], &(*Polohrany)[16], &(*Polohrany)[15], &(*Polohrany)[19]);

	(*Polohrany)[18].set(&(*Vrcholy)[4], &(*Steny)[6], &(*Polohrany)[20], &(*Polohrany)[19], &(*Polohrany)[6]);
	(*Polohrany)[19].set(&(*Vrcholy)[3], &(*Steny)[6], &(*Polohrany)[18], &(*Polohrany)[20], &(*Polohrany)[17]);
	(*Polohrany)[20].set(&(*Vrcholy)[5], &(*Steny)[6], &(*Polohrany)[19], &(*Polohrany)[18], &(*Polohrany)[22]);

	(*Polohrany)[21].set(&(*Vrcholy)[1], &(*Steny)[7], &(*Polohrany)[23], &(*Polohrany)[22], &(*Polohrany)[9]);
	(*Polohrany)[22].set(&(*Vrcholy)[4], &(*Steny)[7], &(*Polohrany)[21], &(*Polohrany)[23], &(*Polohrany)[20]);
	(*Polohrany)[23].set(&(*Vrcholy)[5], &(*Steny)[7], &(*Polohrany)[22], &(*Polohrany)[21], &(*Polohrany)[13]);
	
	

	(*Steny)[0].setEdge(&(*Polohrany)[0]);
	(*Steny)[1].setEdge(&(*Polohrany)[3]);
	(*Steny)[2].setEdge(&(*Polohrany)[6]);
	(*Steny)[3].setEdge(&(*Polohrany)[9]);
	(*Steny)[4].setEdge(&(*Polohrany)[12]);
	(*Steny)[5].setEdge(&(*Polohrany)[15]);
	(*Steny)[6].setEdge(&(*Polohrany)[18]);
	(*Steny)[7].setEdge(&(*Polohrany)[21]);

	octa.setVrcholy(Vrcholy);
	octa.setHrany(Polohrany);
	octa.setSteny(Steny);
	msgBox.setText(u8"Octahedron bol vytvoren?.");
	msgBox.setIcon(QMessageBox::Information);
	msgBox.exec();

}

void ImageViewer::on_rozdel_clicked() {
	QList<Vertex>* Vrcholy = octa.getVrcholy();
	QList<H_Edge>* Polohrany = octa.getHrany();
	QList<Face>* Steny = octa.getSteny();
	double x0 ,x1, x, y0, y1, y, z0, z1, z, d;
	int i, j, stenySize= octa.getStenysize(), polohranySize = octa.getHranysize();
	H_Edge* actualEdge;
	for (i = 0; i < stenySize; i++) {
		actualEdge = (*Steny)[i].getEdge();
		int indexOriginVertex0 = actualEdge->getVOIndex();
		int indexOriginVertex1 = actualEdge->getHrana_next()->getVOIndex();
		int indexOriginVertex2 = actualEdge->getHrana_prev()->getVOIndex();

		x0 = (*Vrcholy)[indexOriginVertex0].getX();			//prvy bod
		x1 = (*Vrcholy)[indexOriginVertex1].getX();
		x = (x0 + x1) / 2.0;
		y0 = (*Vrcholy)[indexOriginVertex0].getY();
		y1 = (*Vrcholy)[indexOriginVertex1].getY();
		y = (y0 + y1) / 2.0;
		z0 = (*Vrcholy)[indexOriginVertex0].getZ();
		z1 = (*Vrcholy)[indexOriginVertex1].getZ();
		z = (z0 + z1) / 2.0;
		/*d = sqrt(x * x + y * y + z * z); //projekcia na jednotkovu kruznicu 
		x = x / d;
		y = y / d;
		z = z / d;*/

		Vertex* A;
		A = new Vertex();
		int indexA = -1;
		(*A).setSur(x, y, z);
		for (j = 0; j < octa.getVrcholysize(); j++) {
			if ((*Vrcholy)[j].getX() == (*A).getX() && (*Vrcholy)[j].getY() == (*A).getY() && (*Vrcholy)[j].getZ() == (*A).getZ()) {
				indexA = j;
				(*A).setIndex(j);
			}
		}
		if (indexA == -1) {
			(*A).setIndex(octa.getVrcholysize());
			Vrcholy->append(*A);
		}


		x0 = (*Vrcholy)[indexOriginVertex0].getX();			//druhy bod
		x1 = (*Vrcholy)[indexOriginVertex2].getX();
		x = (x0 + x1) / 2.0;
		y0 = (*Vrcholy)[indexOriginVertex0].getY();
		y1 = (*Vrcholy)[indexOriginVertex2].getY();
		y = (y0 + y1) / 2.0;
		z0 = (*Vrcholy)[indexOriginVertex0].getZ();
		z1 = (*Vrcholy)[indexOriginVertex2].getZ();
		z = (z0 + z1) / 2.0;
		/*d = sqrt(x * x + y * y + z * z); //projekcia na jednotkovu kruznicu 
		x = x / d;
		y = y / d;
		z = z / d;*/

		Vertex* B;
		B = new Vertex();
		int indexB = -1;
		(*B).setSur(x, y, z);
		for (j = 0; j < octa.getVrcholysize(); j++) {
			if ((*Vrcholy)[j].getX() == (*B).getX() && (*Vrcholy)[j].getY() == (*B).getY() && (*Vrcholy)[j].getZ() == (*B).getZ()) {
				indexB = j;
				(*B).setIndex(j);
			}
		}
		if (indexB == -1) {
			(*B).setIndex(octa.getVrcholysize());
			Vrcholy->append(*B);
		}


		x0 = (*Vrcholy)[indexOriginVertex1].getX();			//treti bod
		x1 = (*Vrcholy)[indexOriginVertex2].getX();
		x = (x0 + x1) / 2.0;
		y0 = (*Vrcholy)[indexOriginVertex1].getY();
		y1 = (*Vrcholy)[indexOriginVertex2].getY();
		y = (y0 + y1) / 2.0;
		z0 = (*Vrcholy)[indexOriginVertex1].getZ();
		z1 = (*Vrcholy)[indexOriginVertex2].getZ();
		z = (z0 + z1) / 2.0;
		/*d = sqrt(x * x + y * y + z * z); //projekcia na jednotkovu kruznicu 
		x = x / d;
		y = y / d;
		z = z / d;*/

		Vertex* C;
		C = new Vertex();
		int indexC = -1;
		(*C).setSur(x, y, z);
		for (j = 0; j < octa.getVrcholysize(); j++) {
			if ((*Vrcholy)[j].getX() == (*C).getX() && (*Vrcholy)[j].getY() == (*C).getY() && (*Vrcholy)[j].getZ() == (*C).getZ()) {
				indexC = j;
				(*C).setIndex(j);
			}
		}
		if (indexC == -1) {
			(*C).setIndex(octa.getVrcholysize());
			Vrcholy->append(*C);
		}

		Face* f0 = new Face();
		Face* f1 = new Face();
		Face* f2 = new Face();
		Face* f3 = new Face();

		H_Edge* e0 = new H_Edge();
		H_Edge* e1 = new H_Edge();
		H_Edge* e2 = new H_Edge();
		H_Edge* e3 = new H_Edge();
		H_Edge* e4 = new H_Edge();
		H_Edge* e5 = new H_Edge();
		H_Edge* e6 = new H_Edge();
		H_Edge* e7 = new H_Edge();
		H_Edge* e8 = new H_Edge();
		H_Edge* e9 = new H_Edge();
		H_Edge* e10 = new H_Edge();
		H_Edge* e11 = new H_Edge();

		(*e0).set(&(*Vrcholy)[indexOriginVertex0], f0, e2, e1, NULL);
		(*Polohrany).append(*e0);
		(*e1).set(&(*A), f0, e0, e2, e9);
		(*Polohrany).append(*e1);
		(*e2).set(&(*B), f0, e1, e0, NULL);
		(*Polohrany).append(*e2);
		(*e3).set(&(*A), f1, e5, e4, NULL);
		(*Polohrany).append(*e3);
		(*e4).set(&(*Vrcholy)[indexOriginVertex1], f1, e3, e5, NULL);
		(*Polohrany).append(*e4);
		(*e5).set(&(*C), f1, e4, e3, e10);
		(*Polohrany).append(*e5);
		(*e6).set(&(*B), f2, e8, e7, e11);
		(*Polohrany).append(*e6);
		(*e7).set(&(*C), f2, e6, e8, NULL);
		(*Polohrany).append(*e7);
		(*e8).set(&(*Vrcholy)[indexOriginVertex2], f2, e7, e6, NULL);
		(*Polohrany).append(*e8);
		(*e9).set(&(*B), f3, e11, e10, e1);
		(*Polohrany).append(*e9);
		(*e10).set(&(*A), f3, e9, e11, e5);
		(*Polohrany).append(*e10);
		(*e11).set(&(*C), f3, e10, e9, e6);
		(*Polohrany).append(*e11);

		(*f0).setEdge(&(*e0));
		(*Steny).append(*f0);
		(*f1).setEdge(&(*e3));
		(*Steny).append(*f1);
		(*f2).setEdge(&(*e6));
		(*Steny).append(*f2);
		(*f3).setEdge(&(*e9));
		(*Steny).append(*f3);
	}
	//projekcia na jednotkovu kruznicu
	for (i = 0; i < octa.getVrcholysize(); i++) {
		x = (*Vrcholy)[i].getX();
		y = (*Vrcholy)[i].getY();
		z = (*Vrcholy)[i].getZ();
		d = sqrt(x * x + y * y + z * z);
		if ((1.0 - d) != 0) {
			(*Vrcholy)[i].setX(x / d);
			(*Vrcholy)[i].setY(y / d);
			(*Vrcholy)[i].setZ(z / d);
		}
	}

	for (i = 0; i < polohranySize; i++) {
		(*Polohrany).removeFirst();
	}
	for (i = 0; i < stenySize; i++) {
		(*Steny).removeFirst();
	}
	octa.setVrcholy(Vrcholy);
	octa.setHrany(Polohrany);
	octa.setSteny(Steny);
	octa.setParove();
	qDebug() << "delenie OK";
}

void ImageViewer::on_imp_clicked() {

	//pridat vycistenie octa
	if (!octa.HisEmpty())
		octa.clear();

	//otvorenie suboru
	QString fileName = QFileDialog::getOpenFileName(this, "Load hedron", "", "Vtk data (*.vtk *.txt);;All files (*)");
	if (fileName.isEmpty()) { return; }
	QFile file;
	file.setFileName(fileName);
	if (!file.open(QIODevice::ReadOnly)) {
		msgBox.setText("Unable to open file.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
	//kontrola uvodnych riadkov
	QString line;
	line = file.readLine();
	if (line != "# vtk DataFile Version 3.0\r\n") {
		msgBox.setText(u8"Hlavi?ka s?boru nie je spr?vna.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
		return;
	}
	line = file.readLine();
	if (line != "vtk output\r\n") {
		msgBox.setText(u8"Hlavi?ka s?boru nie je spr?vna.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
		return;
	}
	line = file.readLine();
	if (line != "ASCII\r\n") {
		msgBox.setText(u8"Hlavi?ka s?boru nie je spr?vna.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
		return;
	}
	line = file.readLine();
	if (line != "DATASET POLYDATA\r\n") {
		msgBox.setText(u8"Hlavi?ka s?boru nie je spr?vna.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
		return;
	}

	//zapis vrcholov
	int i,j,k,g=0; 
	line = file.readLine();
	if (line.split(" ").at(0) != "POINTS")
		return;
	int vrcholySize=line.split(" ").at(1).toInt();
	QList<Vertex>* Vrcholy;
	Vrcholy = new QList<Vertex>;
	Vrcholy->reserve(vrcholySize);
	for (i = 0; i < vrcholySize; i++) {
		*Vrcholy << Vertex();
	}
	for (i = 0; i < vrcholySize; i++) {
		line = file.readLine();
		(*Vrcholy)[i].set(i, line.split(" ").at(0).toFloat(), line.split(" ").at(1).toFloat(), line.split(" ").at(2).toFloat());
	}

	//zapis hran a stien
	line = file.readLine();
	if (line.split(" ").at(0) != "LINES")
		return;
	int polohranySize = line.split(" ").at(1).toInt();
	QList<H_Edge>* Polohrany;
	Polohrany = new QList<H_Edge>;
	Polohrany->reserve(polohranySize*2);
	for (i = 0; i < polohranySize * 2 ; i++) {
		*Polohrany << H_Edge();
	}
	for (i = 0; i < polohranySize * 2; i += 2) {
		line = file.readLine();
		int prvy = line.split(" ").at(1).toInt();
		int druhy = line.split(" ").at(2).toInt();
		(*Polohrany)[i].setVert(&(*Vrcholy)[prvy], &(*Vrcholy)[druhy]);
		(*Polohrany)[i + 1].setVert(&(*Vrcholy)[druhy], &(*Vrcholy)[prvy]);
		(*Polohrany)[i + 1].setPair(&(*Polohrany)[i]);
		(*Polohrany)[i].setPair(&(*Polohrany)[i + 1]);
		}
	line = file.readLine();
	if (line.split(" ").at(0) != "POLYGONS")
		return;
	int stenySize=line.split(" ").at(1).toInt();
	QList<int> VrcholyStien;
	QList<Face>* Steny;
	Steny = new QList<Face>;
	Steny->reserve(stenySize);
	for (i = 0; i < stenySize; i++) {
		*Steny << Face();
	}
	for (i = 0; i < stenySize; i++) {
		int a, b, c;
		line = file.readLine();
		a = line.split(" ").at(1).toInt();
		b = line.split(" ").at(2).toInt();
		c = line.split(" ").at(3).toInt();
		VrcholyStien.append(a);
		VrcholyStien.append(b);
		VrcholyStien.append(c);
	int n = -1, p = -1;
		for (j = 0; j < polohranySize * 2; j++) {
			if (((*Polohrany)[j].getVOIndex() == a && (*Polohrany)[j].getVEIndex() == b)) {
				g++;
				(*Polohrany)[j].setFace(&(*Steny)[i]);
				for (k = 0; k < polohranySize * 2; k++) {
					if ((*Polohrany)[k].getVOIndex() == c && (*Polohrany)[k].getVEIndex() == a) {
						(*Polohrany)[j].setEdgePrev(&(*Polohrany)[k]);
						(*Polohrany)[k].setEdgeNext(&(*Polohrany)[j]);
						(*Polohrany)[k].setFace(&(*Steny)[i]);
						(*Steny)[i].setEdge(&(*Polohrany)[j]);
						n = k;
						if (p >= 0) {
							(*Polohrany)[n].setEdgePrev(&(*Polohrany)[p]);
							(*Polohrany)[p].setEdgeNext(&(*Polohrany)[n]);
						}
					}
					else if ((*Polohrany)[k].getVOIndex() == b && (*Polohrany)[k].getVEIndex() == c) {
						(*Polohrany)[j].setEdgeNext(&(*Polohrany)[k]);
						(*Polohrany)[k].setEdgePrev(&(*Polohrany)[j]);
						(*Polohrany)[k].setFace(&(*Steny)[i]);
						p = k;
						if (n >= 0) {
							(*Polohrany)[n].setEdgePrev(&(*Polohrany)[p]);
							(*Polohrany)[p].setEdgeNext(&(*Polohrany)[n]);
						}
					}
				}
			}
		}
	}
	int a, b, c;

	
	/*//kontrolne vypisy
	for (i = 0; i < polohranySize*2; i++)
		qDebug() <<  (*Polohrany)[i].getVOIndex() << (*Polohrany)[i].getHrana_next()->getVOIndex() << (*Polohrany)[i].getHrana_prev()->getVOIndex();

	for (i = 0; i < stenySize; i++)
		qDebug() << (*Steny)[i].getEdge();*/

	file.close();
	//zapis do octa
	octa.setVrcholy(Vrcholy);
	octa.setHrany(Polohrany);
	octa.setSteny(Steny); 

	msgBox.setText(u8"Import bol ?spe?n?.");
	msgBox.setIcon(QMessageBox::Information);
	msgBox.exec();
}

void ImageViewer::on_exp_clicked() {
	int i;
	//pridat podmienku, ze ak nie je octa empty
	if (octa.HisEmpty()) {
		msgBox.setText(u8"?tvar je pr?zdny.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
		return;
	}

	QFile file("out2.vtk");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);
	out << "# vtk DataFile Version 3.0\n";
	out << "vtk output\n";
	out << "ASCII\n";
	out << "DATASET POLYDATA\n";
	out << "POINTS " << octa.getVrcholysize() << " float\n";
	for (i = 0; i < octa.getVrcholysize(); i++) {
		out << octa.printSur(i) <<"\n";
	}
	out << "LINES " << octa.getHranysize() / 2 << " " << octa.getHranysize() / 2 * 3 << "\n";
	for (i = 0; i < octa.getHranysize(); i++) {
		if(octa.getHranaOIndex(i)<=octa.getPairHranaOIndex(i))
			out << "2 " <<octa.printHrana(i) << "\n";
	}
	out << "POLYGONS " << octa.getStenysize() << " " << octa.getStenysize() * 4 << "\n";
	for (i = 0; i < octa.getStenysize(); i++) {
		out << "3 " << octa.printStena(i) << "\n";
	}
	file.close();
	QString msgText=u8"?tvar bol ulo?en? do s?boru ";
	msgText.append(file.fileName());
	msgBox.setText(msgText);
	msgBox.setIcon(QMessageBox::Information);
	msgBox.exec();
}