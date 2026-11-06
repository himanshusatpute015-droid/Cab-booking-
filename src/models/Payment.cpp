#include "models/Payment.h"

#include <ctime>

#include "core/Exceptions.h"
#include "core/Utils.h"

namespace cab {

const double CardPayment::FEE_RATE = 0.02;

std::string Payment::makeTransactionId(const std::string& prefix, int bookingId) const {
    long stamp = static_cast<long>(std::time(nullptr) % 1000000);
    return prefix + "-" + utils::toString(bookingId) + "-" + utils::toString(stamp);
}

std::string CashPayment::pay(double amount, int bookingId) const {
    if (amount <= 0) throw PaymentException("Nothing to pay.");
    return makeTransactionId("CASH", bookingId);
}

std::string UpiPayment::pay(double amount, int bookingId) const {
    if (amount <= 0) throw PaymentException("Nothing to pay.");
    if (amount > 100000) throw PaymentException("UPI transactions are limited to Rs 1,00,000.");
    return makeTransactionId("UPI", bookingId);
}

std::string CardPayment::pay(double amount, int bookingId) const {
    if (amount <= 0) throw PaymentException("Nothing to pay.");
    return makeTransactionId("CARD", bookingId);
}

std::string WalletPayment::pay(double amount, int bookingId) const {
    if (amount <= 0) throw PaymentException("Nothing to pay.");
    return makeTransactionId("WAL", bookingId);
}

std::unique_ptr<Payment> PaymentFactory::create(const std::string& method) {
    std::string key = utils::toLower(utils::trim(method));
    if (key == "cash") return std::unique_ptr<Payment>(new CashPayment());
    if (key == "upi") return std::unique_ptr<Payment>(new UpiPayment());
    if (key == "card") return std::unique_ptr<Payment>(new CardPayment());
    if (key == "wallet") return std::unique_ptr<Payment>(new WalletPayment());
    throw ValidationException("Unknown payment method: " + method);
}

std::vector<std::string> PaymentFactory::methods() { return {"UPI", "Cash", "Card", "Wallet"}; }

}  // namespace cab
