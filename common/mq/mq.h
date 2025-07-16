#ifndef MQ_H
#define MQ_H
 
#include <QObject>
#include <QString>

struct mq_attr;

/**
 * @brief класс для работы с одним экземпляром очереди POSIX
 *
 * в конструкторе необходимо указать имя очереди без символа '/'.
 * при создании объект еще не может работать с очередью, сначала нужно к ней подключиться или создать ее.
 * объект может подключиться(отключиться) к существующей очереди POSIX.
 * объект может записывать/считывать данные в/из очередь.
 * объект может создать новую очередь POSIX, а также удалить ее из системы.
 */
class MQ : public QObject
{
    Q_OBJECT

public:
    enum MQState {
        mqsDeinit = 221,
        mqsOpened,
        mqsClosed,
        mqsCreated,
        mqsNotFound,
        mqsInvalid
    };

private:
    struct MQData {
        QString  mq_name;            ///< имя очереди POSIX
        int 	 size   = -1;	     ///< текущий размер очереди
        int		 handle = -99;	     ///< дескриптор очереди posix
        MQState  state  = mqsDeinit; ///< MQState enum element
        int 	 mode   = 0; 	     ///< IODevice enum element
        mq_attr *attrs  = Q_NULLPTR; ///< current attributes
    };

public:
    explicit MQ(const QString& name, QObject *parent = Q_NULLPTR);
    
    /// имя очереди POSIX
    QString name() const {
        return d.mq_name;
    }

    /// текущий размер всех сообщений находящихся в очереди
    int size() const {
        return d.size;
    }

    bool invalid() const {
        return (d.handle <= 0);
    }

    /// дескриптор очереди, к которой подключен объект
    int handle() const {
        return d.handle;
    }

    bool isOpened() const {
        return (d.state == mqsOpened || d.state == mqsCreated);
    }

    bool isNotFound() const {
        return (d.state == mqsNotFound);
    }

    bool isDeinit() const {
        return (d.state == mqsDeinit);
    }

    QString strMode() const;
    QString strState() const;
    QString strStatus() const;
    QString strAttrs() const;

    /// обновить информацию об текущем состоянии очереди
    void updateAttrs();

    /// признак наличия сообщений в очереди
    bool hasMsg() const;

    /// подключиться к существующей очереди POSIX
    bool tryOpen(int mode);

    /// отключиться от очереди POSIX
    bool tryClose();

    /// создать очередь POSIX (после создания объкт будет к ней подключен)
    bool tryCreate(int mode, quint32 msg_size);

    /// уничтожить из системы очередь POSIX
    bool tryDestroy();

    /// отправить сообщение(массив байт) в очередь POSIX (объект должен быть подключен к очереди)
    bool trySendMsg(const QByteArray& ba);

    /// считать сообщение из очереди, в случае ошибки ba будет пустой
    QByteArray tryReadMsg();

    /// очистить очередь, т.е. просто считать все сообщения (если они есть) из очереди в никуда.
    bool tryClearAllMsgs();

    void resetState();

Q_SIGNALS:
    void logMessage(const QString &msgHandler, const QString &msg);
    void logError(const QString &msgHandler, const QString &msg);
    	
private:
    /**
     * @brief проверить наличие файла-очереди POSIX для этого объекта в каталоге /dev/mqueue.
     *
     * если такая очередь не существует, а статус объекта висит как ОТКРЫТА то функция пытается ее закрыть.
     * после выполнения этой функции нужно выполнить: if (isNotFound()) { to do }
     * @param check_invalid
     */
    void checkQueueFile(bool check_invalid = true);

    /// признак наличия файла-очереди /dev/mqueue/mq_name
    bool existPosixFile() const;

private:
    MQData d;
};
 
#endif
 
