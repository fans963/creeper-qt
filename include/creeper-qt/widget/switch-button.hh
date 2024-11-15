#pragma once

#include "../setting/theme.hh"
#include "../utility/pid.hh"
#include "../widget/widget.hh"

#include <qabstractbutton.h>
#include <qevent.h>
#include <qpainter.h>
#include <qpropertyanimation.h>
#include <qtimer.h>

namespace creeper {

// thanks to https://stackoverflow.com/a/38102598
// modify a lot
class AbstractSwitchButton : public Extension<QAbstractButton> {
    Q_OBJECT
public:
    AbstractSwitchButton(QWidget* parent = nullptr)
        : Extension(parent) {
        connect(&animationTimer_, &QTimer::timeout, [this] { update(); });
        reloadTheme();
    }

    void setFixedSize(QSize size) {
        progress_ = switchStatus_
            ? size.width() - size.height() / 2
            : size.height() / 2;
        QAbstractButton::setFixedSize(size);
    }

    void setSwitchStatus(bool switchStatus) {
        switchStatus_ = switchStatus;
        update();
    }

    bool switched() const { return switchStatus_; }

    void reloadTheme() override {
        lightPrimary_ = Theme::color("primary200");
        heavyPrimary_ = Theme::color("primary400");
    }

protected:
    void mouseReleaseEvent(QMouseEvent* event) override {
        if (event->button() & Qt::LeftButton) {
            switchStatus_ = !switchStatus_;
            if (!animationTimer_.isActive())
                animationTimer_.start(refreshIntervalMs_);
        }
        QAbstractButton::mouseReleaseEvent(event);
    }

    void enterEvent(QEnterEvent* event) override {
        setCursor(Qt::PointingHandCursor);
        QAbstractButton::enterEvent(event);
    }

    QTimer animationTimer_;

    uint32_t lightGrey_ = 0xdddddd;
    uint32_t heavyGrey_ = 0xaaaaaa;
    uint32_t lightPrimary_ = 0x7c55bb;
    uint32_t heavyPrimary_ = 0x5d34a9;

    bool switchStatus_ = false;
    double progress_ = 0;
};

class ConvexSwitchButton : public AbstractSwitchButton {
    Q_OBJECT
public:
    ConvexSwitchButton(QWidget* parent = nullptr)
        : AbstractSwitchButton(parent) { }

protected:
    void paintEvent(QPaintEvent* event) override {
        const auto enabled = isEnabled();
        const auto h = height();
        const auto w = width();

        const auto r0 = h * 0.45;
        const auto r1 = r0 * 0.75;

        const auto leftCenter = QPoint(h / 2, h / 2);
        const auto rightCenter = QPoint(w - h / 2, h / 2);

        const double target = switchStatus_ ? w - h / 2 : h / 2;
        const auto currentCenter = QPoint(progress_, h / 2);

        progress_ = updateWithPid(progress_, target, 0.1);
        if (std::abs(progress_ - target) < 0.1)
            animationTimer_.stop();

        auto painter = QPainter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        painter.setOpacity(1.0);

        if (!enabled) {
            const auto p0 = leftCenter - QPoint(r1, r1);
            const auto p1 = rightCenter + QPoint(r1, r1);
            painter.setBrush({ heavyGrey_ + 0x333333 });
            painter.drawRoundedRect(QRect(p0, p1), r1, r1);

            const auto p2 = currentCenter - QPoint(r0, r0);
            const auto p3 = currentCenter + QPoint(r0, r0);
            painter.setBrush({ heavyGrey_ });
            painter.drawEllipse(QRect(p2, p3));
            return;
        }

        painter.setOpacity(0.75);

        const auto lineLeft = leftCenter - QPoint(r1, r1);
        const auto lineCurrentLeft = currentCenter + QPoint(r1, r1);
        painter.setBrush({ lightPrimary_ });
        painter.drawRoundedRect(QRect(lineLeft, lineCurrentLeft), r1, r1);

        const auto lineRight = rightCenter + QPoint(r1, r1);
        const auto lineCurrentRight = currentCenter - QPoint(r1, r1);
        painter.setBrush({ lightGrey_ });
        painter.drawRoundedRect(QRect(lineCurrentRight, lineRight), r1, r1);

        painter.setOpacity(1.0);

        const auto ballLeft = currentCenter - QPoint(r0, r0);
        const auto ballRight = currentCenter + QPoint(r0, r0);
        const auto ballColor = switchStatus_ ? heavyPrimary_ : lightGrey_;
        painter.setBrush({ ballColor });
        painter.drawEllipse(QRect(ballLeft, ballRight));
    }
};

class ConcaveSwitchButton : public AbstractSwitchButton {
    Q_OBJECT
public:
    ConcaveSwitchButton(QWidget* parent = nullptr)
        : AbstractSwitchButton(parent) { }

protected:
    void paintEvent(QPaintEvent* event) override {
        const auto enabled = isEnabled();
        const auto h = height();
        const auto w = width();

        const auto r0 = h * 0.45;
        const auto r1 = r0 * 0.75;

        const auto leftCenter = QPoint(h / 2, h / 2);
        const auto rightCenter = QPoint(w - h / 2, h / 2);

        const double target = switchStatus_ ? w - h / 2 : h / 2;
        const auto currentCenter = QPoint(progress_, h / 2);

        progress_ = updateWithPid(progress_, target, 0.1);
        if (std::abs(progress_ - target) < 0.1)
            animationTimer_.stop();

        auto painter = QPainter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        painter.setOpacity(1.0);

        if (!enabled) {
            const auto p0 = leftCenter - QPoint(r0, r0);
            const auto p1 = rightCenter + QPoint(r0, r0);
            painter.setBrush({ heavyGrey_ + 0x333333 });
            painter.drawRoundedRect(QRect(p0, p1), r1, r1);

            const auto p2 = currentCenter - QPoint(r1, r1);
            const auto p3 = currentCenter + QPoint(r1, r1);
            painter.setBrush({ heavyGrey_ });
            painter.drawEllipse(QRect(p2, p3));
            return;
        }

        painter.setOpacity(1);

        const auto lineLeft = leftCenter - QPoint(r0, r0);
        const auto lineRight = rightCenter + QPoint(r0, r0);

        painter.setBrush({ switchStatus_ ? heavyPrimary_ : heavyGrey_ });
        painter.drawRoundedRect(QRect(lineLeft, lineRight), r0, r0);

        const auto border = r0 * 0.2;
        const auto lineInerLeft = lineLeft + QPoint(border, border);
        const auto lineInerRight = lineRight - QPoint(border, border);
        const auto r2 = r0 - border;

        if (!switchStatus_) {
            painter.setBrush({ lightGrey_ });
            painter.drawRoundedRect(QRect(lineInerLeft, lineInerRight), r2, r2);
        }

        const auto ballLeft = currentCenter - QPoint(r1, r1);
        const auto ballRight = currentCenter + QPoint(r1, r1);
        const auto ballColor = switchStatus_ ? 0xffffff : heavyGrey_;
        painter.setBrush({ ballColor });
        painter.drawEllipse(QRect(ballLeft, ballRight));
    }
};

}