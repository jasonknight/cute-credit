# Sending an order
Using the PAY command, include a : separated list of values ind, cashier reference, and account number like so
Usage: 
  PAY ID PRICE.CENTS:IND:CSHIER:ACCNT
Example:
  PAY O1234 32.95:U:23:01

Possible Responses:
PAID ID TEXT
FAIL ID TEXT




PAY ORDER1234 32.95
RETV ORDER1234
QURY 0  
EODY 0 0
EXPT 0 2011-12-15 2011-12-13
PAID ORDER1234 PTEXT
FAIL ORDER1234 ErrorText
