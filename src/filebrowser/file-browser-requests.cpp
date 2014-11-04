#include "file-browser-requests.h"

#include <jansson.h>
#include <QtNetwork>
#include <QScopedPointer>

#include "account.h"
#include "api/api-error.h"
#include "seaf-dirent.h"

namespace {

const char kGetDirentsUrl[] = "api2/repos/%1/dir/";
const char kGetFilesUrl[] = "api2/repos/%1/file/";
const char kGetFileSharedLinkUrl[] = "api2/repos/%1/file/shared-link/";
const char kGetFileUploadUrl[] = "api2/repos/%1/upload-link/";
const char kGetFileUpdateUrl[] = "api2/repos/%1/update-link/";
const char kGetStarredFilesUrl[] = "api2/starredfiles/";
//const char kGetFileFromRevisionUrl[] = "api2/repos/%1/file/revision/";
//const char kGetFileDetailUrl[] = "api2/repos/%1/file/detail/";
//const char kGetFileHistoryUrl[] = "api2/repos/%1/file/history/";

} // namespace


GetDirentsRequest::GetDirentsRequest(const Account& account,
                                     const QString& repo_id,
                                     const QString& path)
    : SeafileApiRequest (account.getAbsoluteUrl(QString(kGetDirentsUrl).arg(repo_id)),
                         SeafileApiRequest::METHOD_GET, account.token)
{
    setUrlParam("p", path);

    repo_id_ = repo_id;
    path_ = path;
}

void GetDirentsRequest::requestSuccess(QNetworkReply& reply)
{
    json_error_t error;
    QString dir_id = reply.rawHeader("oid");
    if (dir_id.length() != 40) {
        emit failed(ApiError::fromHttpError(500));
        return;
    }

    json_t *root = parseJSON(reply, &error);
    if (!root) {
        qDebug("GetDirentsRequest: failed to parse json:%s\n", error.text);
        emit failed(ApiError::fromJsonError());
        return;
    }

    QScopedPointer<json_t, JsonPointerCustomDeleter> json(root);

    QList<SeafDirent> dirents;
    dirents = SeafDirent::listFromJSON(json.data(), &error);
    emit success(dirents);
}

GetFileDownloadLinkRequest::GetFileDownloadLinkRequest(const Account &account,
                                                       const QString &repo_id,
                                                       const QString &path)
    : SeafileApiRequest(
          account.getAbsoluteUrl(QString(kGetFilesUrl).arg(repo_id)),
          SeafileApiRequest::METHOD_GET, account.token)
{
    setUrlParam("p", path);
}

void GetFileDownloadLinkRequest::requestSuccess(QNetworkReply& reply)
{
    QString reply_content(reply.readAll());
    QString oid;

    if (reply.hasRawHeader("oid"))
        oid = reply.rawHeader("oid");

    do {
        if (reply_content.size() <= 2)
            break;
        reply_content.remove(0, 1);
        reply_content.chop(1);
        QUrl new_url(reply_content);

        if (!new_url.isValid())
            break;

        file_id_ = oid;
        emit success(reply_content);
        return;
    } while (0);
    emit failed(ApiError::fromHttpError(500));
}

GetSharedLinkRequest::GetSharedLinkRequest(const Account &account,
                                           const QString &repo_id,
                                           const QString &path,
                                           bool is_file)
    : SeafileApiRequest(
          account.getAbsoluteUrl(QString(kGetFileSharedLinkUrl).arg(repo_id)),
          SeafileApiRequest::METHOD_PUT, account.token)
{
    setFormParam("type", is_file ? "f" : "d");
    setFormParam("p", path);
}

void GetSharedLinkRequest::requestSuccess(QNetworkReply& reply)
{
    QString reply_content(reply.rawHeader("Location"));

    emit success(reply_content);
}

CreateDirentRequest::CreateDirentRequest(const Account &account,
                                         const QString &repo_id,
                                         const QString &path)
    : SeafileApiRequest(
          account.getAbsoluteUrl(QString(kGetDirentsUrl).arg(repo_id)),
          SeafileApiRequest::METHOD_GET, account.token)
{
    setUrlParam("p", path);

    setFormParam("operation", "mkdir");
}

void CreateDirentRequest::requestSuccess(QNetworkReply& reply)
{
    QString reply_content(reply.readAll());

    emit success(reply_content);
}

RenameDirentRequest::RenameDirentRequest(const Account &account,
                                         const QString &repo_id,
                                         const QString &path,
                                         const QString &new_path)
    : SeafileApiRequest(
          account.getAbsoluteUrl(QString(kGetDirentsUrl).arg(repo_id)),
          SeafileApiRequest::METHOD_POST, account.token)
{
    setUrlParam("p", path);

    setFormParam("operation", "rename");
    setFormParam("newname", new_path);
}

void RenameDirentRequest::requestSuccess(QNetworkReply& reply)
{
    emit success();
}

RemoveDirentRequest::RemoveDirentRequest(const Account &account,
                                         const QString &repo_id,
                                         const QString &path)
    : SeafileApiRequest(
          account.getAbsoluteUrl(QString(kGetDirentsUrl).arg(repo_id)),
          SeafileApiRequest::METHOD_DELETE, account.token)
{
    setUrlParam("p", path);
}

void RemoveDirentRequest::requestSuccess(QNetworkReply& reply)
{
    emit success();
}

GetFileUploadLinkRequest::GetFileUploadLinkRequest(const Account &account,
                                                   const QString &repo_id)
    : SeafileApiRequest(
          account.getAbsoluteUrl(QString(kGetFileUploadUrl).arg(repo_id)),
          SeafileApiRequest::METHOD_GET, account.token)
{
}

void GetFileUploadLinkRequest::requestSuccess(QNetworkReply& reply)
{
    QString reply_content(reply.readAll());

    do {
        if (reply_content.size() <= 2)
            break;
        reply_content.remove(0, 1);
        reply_content.chop(1);
        QUrl new_url(reply_content);

        if (!new_url.isValid())
            break;

        emit success(reply_content);
        return;
    } while (0);
    emit failed(ApiError::fromHttpError(500));
}

GetFileUpdateLinkRequest::GetFileUpdateLinkRequest(const Account &account,
                                                   const QString &repo_id)
    : SeafileApiRequest(
          account.getAbsoluteUrl(QString(kGetFileUpdateUrl).arg(repo_id)),
          SeafileApiRequest::METHOD_GET, account.token)
{
}

void GetFileUpdateLinkRequest::requestSuccess(QNetworkReply& reply)
{
    QString reply_content(reply.readAll());

    do {
        if (reply_content.size() <= 2)
            break;
        reply_content.remove(0, 1);
        reply_content.chop(1);
        QUrl new_url(reply_content);

        if (!new_url.isValid())
            break;

        emit success(reply_content);
        return;
    } while (0);
    emit failed(ApiError::fromHttpError(500));
}

RenameFileRequest::RenameFileRequest(const Account &account,
                                     const QString &repo_id,
                                     const QString &path,
                                     const QString &new_name)
    : SeafileApiRequest(
          account.getAbsoluteUrl(QString(kGetFilesUrl).arg(repo_id)),
          SeafileApiRequest::METHOD_POST, account.token)
{
    setUrlParam("p", path);

    setFormParam("operation", "rename");
    setFormParam("newname", new_name);
}

void RenameFileRequest::requestSuccess(QNetworkReply& reply)
{
    emit success();
}

MoveFileRequest::MoveFileRequest(const Account &account,
                                 const QString &repo_id,
                                 const QString &path,
                                 const QString &new_repo_id,
                                 const QString &new_path)
    : SeafileApiRequest(
          account.getAbsoluteUrl(QString(kGetFilesUrl).arg(repo_id)),
          SeafileApiRequest::METHOD_POST, account.token)
{
    setUrlParam("p", path);

    setFormParam("operation", "move");
    setFormParam("dst_repo", new_repo_id);
    setFormParam("dst_dir", new_path);
}

void MoveFileRequest::requestSuccess(QNetworkReply& reply)
{
    emit success();
}

RemoveFileRequest::RemoveFileRequest(const Account &account,
                                     const QString &repo_id,
                                     const QString &path)
    : SeafileApiRequest(
          account.getAbsoluteUrl(QString(kGetFilesUrl).arg(repo_id)),
          SeafileApiRequest::METHOD_DELETE, account.token)
{
    setUrlParam("p", path);
}

void RemoveFileRequest::requestSuccess(QNetworkReply& reply)
{
    emit success();
}

StarFileRequest::StarFileRequest(const Account &account,
                                 const QString &repo_id,
                                 const QString &path)
    : SeafileApiRequest(
          account.getAbsoluteUrl(kGetStarredFilesUrl),
          SeafileApiRequest::METHOD_POST, account.token)
{
    setFormParam("repo_id", repo_id);
    setFormParam("p", path);
}

void StarFileRequest::requestSuccess(QNetworkReply& reply)
{
    emit success();
}

UnstarFileRequest::UnstarFileRequest(const Account &account,
                                     const QString &repo_id,
                                     const QString &path)
    : SeafileApiRequest(
          account.getAbsoluteUrl(kGetStarredFilesUrl),
          SeafileApiRequest::METHOD_DELETE, account.token)
{
    setUrlParam("repo_id", repo_id);
    setUrlParam("p", path);
}

void UnstarFileRequest::requestSuccess(QNetworkReply& reply)
{
    emit success();
}

