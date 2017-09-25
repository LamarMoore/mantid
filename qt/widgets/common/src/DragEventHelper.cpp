// Compile on everything that is not OSX
#if !defined(__APPLE__)
#include "MantidQtWidgets/Common/DragEventHelper.h"

#include <QStringList>
#include <QUrl>

using namespace MantidQt::MantidWidgets;

/** Extract a list of file names from a drop event.
 *
 * @param event :: the event to extract file names from
 * @return a list of file names as a QStringList
 */
QStringList DragEventHelper::getFileNames(const QDropEvent* event) {
    QStringList filenames;
    const auto mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
      const auto urlList = mimeData->urls();
      for (const auto &url :urlList) {
        const auto fName = url.toLocalFile();
        if (fName.size() > 0) {
          filenames.append(fName);
        }
      }
    }
    return filenames;
}
#endif
