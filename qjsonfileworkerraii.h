#pragma once

#include <QtCore>

class QJsonFileWorkerRAII
{
public:
    QJsonFileWorkerRAII(const QString& path, QIODevice::OpenMode ioFlags)
        : m_file{ path }
    {
        if (!m_file.open(ioFlags))
            qCritical() << QString{ "[QJsonFileWorkerRAII::QJsonFileWorkerRAII] Failed to open file: %1" }.arg(path);
        
        if (ioFlags.testFlag(QIODevice::ReadOnly))
        {
            QJsonParseError error{ 0, QJsonParseError::NoError };
            m_document = QJsonDocument::fromJson(m_file.readAll(), &error);
            if (error.error != QJsonParseError::NoError)
                qFatal() << error.errorString().toStdString();
        }
    }
    
    template <typename T>
    T load() const
    {
        T result;
        loadImpl(result);
        return result;
    }
    
    template <typename T>
    void save(const T& value)
    {
        saveImpl(value);
        m_file.write(m_document.toJson());
        m_file.flush();
    }
    
protected:
    template <typename T>
    void loadImpl(T& value) const
    {
        if (!m_document.isObject())
            qCritical() << "[QJsonFileWorkerRAII::loadImpl] Invalid data";
        
        value = static_cast<T>(m_document.object());
    }
    
    template <template <typename> typename C, typename T>
    void loadImpl(C<T>& value) const
    {
        if (!m_document.isArray())
            qCritical() << "[QJsonFileWorkerRAII::loadImpl] Invalid data";
        const auto array = m_document.array();
        for (const auto& obj : array)
            value.push_back(static_cast<T>(obj.toObject()));
    }
    
    template <typename T>
    void saveImpl(const T& value)
    {
        m_document = QJsonDocument(static_cast<QJsonObject>(value));
    }
    
    template <template <typename> typename C, typename T>
    void saveImpl(const C<T>& value)
    {
        QJsonArray array;
        for (const auto& item : value)
            array.append(static_cast<QJsonObject>(item));
        m_document = QJsonDocument(array);
    }
    
private:
    QFile m_file;
    QJsonDocument m_document;
};
