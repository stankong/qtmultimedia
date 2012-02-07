/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

/*!
    \class QAudioProbe
    \inmodule QtMultimedia

    \ingroup multimedia
    \ingroup multimedia_video

    \brief The QAudioProbe class allows you to monitor audio being played or recorded.

    \code
        QAudioRecorder *recorder = new QAudioRecorder();
        QAudioProbe *probe = new QAudioProbe;

        // ... configure the audio recorder (skipped)

        connect(probe, SIGNAL(audioBufferProbed(QAudioBuffer)), this, SLOT(processBuffer(QAudioBuffer)));

        probe->setSource(recorder); // Returns true, hopefully.

        recorder->record(); // Now we can do things like calculating levels or performing an FFT
    \endcode

    \sa QVideoProbe, QMediaPlayer, QCamera
*/

#include "qaudioprobe.h"
#include "qmediaaudioprobecontrol.h"
#include "qmediaservice.h"
#include "qmediarecorder.h"
#include "qsharedpointer.h"

QT_BEGIN_NAMESPACE

class QAudioProbePrivate {
public:
    QWeakPointer<QMediaObject> source;
    QWeakPointer<QMediaAudioProbeControl> probee;
};

/*!
    Creates a new QAudioProbe class.  After setting the
    source to monitor with \l setSource(), the \l audioBufferProbed()
    signal will be emitted when audio buffers are flowing in the
    source media object.
 */
QAudioProbe::QAudioProbe(QObject *parent)
    : QObject(parent)
    , d(new QAudioProbePrivate)
{
}

/*!
    Destroys this probe and disconnects from any
    media object.
 */
QAudioProbe::~QAudioProbe()
{
    if (d->source) {
        // Disconnect
        if (d->probee)
            disconnect(d->probee.data(), SIGNAL(audioBufferProbed(QAudioBuffer)), this, SIGNAL(audioBufferProbed(QAudioBuffer)));
        d->source.data()->service()->releaseControl(d->probee.data());
    }
}

/*!
    Sets the media object to monitor to \a source.

    If \a source is zero, this probe will be deactivated
    and this function wil return true.

    If the media object does not support monitoring
    audio, this function will return false.

    The previous object will no longer be monitored.
    Passing in the same object will be ignored, but
    monitoring will continue.
 */
bool QAudioProbe::setSource(QMediaObject *source)
{
    // Need to:
    // 1) disconnect from current source if necessary
    // 2) see if new one has the probe control
    // 3) connect if so
    if (source != d->source.data()) {
        if (d->source) {
            Q_ASSERT(d->probee);
            disconnect(d->probee.data(), SIGNAL(audioBufferProbed(QAudioBuffer)), this, SIGNAL(audioBufferProbed(QAudioBuffer)));
            d->source.data()->service()->releaseControl(d->probee.data());
            d->source.clear();
            d->probee.clear();
        }

        if (source) {
            QMediaService *service = source->service();
            if (service) {
                d->probee = service->requestControl<QMediaAudioProbeControl*>();
            }

            if (d->probee) {
                connect(d->probee.data(), SIGNAL(audioBufferProbed(QAudioBuffer)), this, SIGNAL(audioBufferProbed(QAudioBuffer)));
                d->source = source;
            }
        }
    }

    return (!source || d->probee != 0);
}

/*!
    Starts monitoring the given \a mediaRecorder.

    If \a source is zero, this probe will be deactivated
    and this function wil return true.

    If the media recorder instance does not support monitoring
    audio, this function will return false.

    Any previously monitored objects will no longer be monitored.
    Passing in the same object will be ignored, but
    monitoring will continue.
 */
bool QAudioProbe::setSource(QMediaRecorder *mediaRecorder)
{
    QMediaObject *source = mediaRecorder ? mediaRecorder->mediaObject() : 0;
    bool result = setSource(source);

    if (!mediaRecorder)
        return true;

    if (mediaRecorder && !source)
        return false;

    return result;
}

/*!
    Returns true if this probe is monitoring something, or false otherwise.

    The source being monitored does not need to be active.
 */
bool QAudioProbe::isActive() const
{
    return d->probee != 0;
}

QT_END_NAMESPACE