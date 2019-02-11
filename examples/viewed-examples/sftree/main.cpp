#include <memory>
#include <string>
#include <string_view>
#include <functional>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <QtWidgets/QTreeView>
#include <QtWidgets/QApplication>

#include "TestTreeModel.hqt"
#include "TestTreeView.hqt"

std::vector<test_tree_entity> generate_data()
{
	std::vector<test_tree_entity> data =
	{
	    {"folder/file1.txt",          "text-descr1", 1},
	    {"folder/file2.txt",          "text-descr2", 2},
	    {"folder/file3.txt",          "text-descr3", 3},
	    {"dir/file1.sft",             "text-descr4", 4},
	    {"dir/prox/dir.txt",          "text-descr5", 5},
	    {"ops.sh",                    "text-descr6", 6},
	    {"westworld.mkv",             "text-descr7", 7},
	    {"folder/sup/file3.txt",      "text-descr8", 8},
	    {"folder/sup/inner/file.txt", "text-descr9", 9},
	};

	return data;
}

std::vector<test_tree_entity> generate_update_data()
{
	std::vector<test_tree_entity> data =
	{
	    {"dir/file1.sft",            "updated-text-descr4", 44},
	    {"dir/prox/dir.txt",         "updated-text-descr5", 55},

	    {"upsershalt/ziggaman.txt",  "new-text-1", 10},
	    {"summer-bucket",            "new-text-2", 11},
	};

	return data;
}


int main(int argc, char * argv[])
{
	using namespace std;

	Q_INIT_RESOURCE(QtTools);

	QtTools::QtRegisterStdString();
	//QtTools::QtRegisterStdChronoTypes();


	QApplication qapp(argc, argv);

#ifdef Q_OS_WIN
	// On windows the highlighted colors for inactive widgets are the
	// same as non highlighted colors.This is a regression from Qt 4.
	// https://bugreports.qt-project.org/browse/QTBUG-41060
	auto palette = qapp.palette();
	palette.setColor(QPalette::Inactive, QPalette::Highlight, palette.color(QPalette::Active, QPalette::Highlight));
	palette.setColor(QPalette::Inactive, QPalette::HighlightedText, palette.color(QPalette::Active, QPalette::HighlightedText));
	qapp.setPalette(palette);
#endif

	TestTreeView view;
	TestTreeModel * model = new TestTreeModel(&view);
	view.SetModel(model);

	auto data = generate_data();
	model->assign(std::move(data));

	view.show();
	//view.ResizeColumnsToContents();

	return qapp.exec();
}



//	auto model = std::make_shared<test_model>();


//	//QTableView view;
//	//view.setModel(model.get());
//	FileTreeView view;
//	view.SetModel(model);
//	QtTools::ResizeColumnsToContents(view.GetTreeView());


