//
// Created by viceda-s on 12/01/25.
//

#include "Bureaucrat.hpp"

void printSeparator(const std::string& title)
{
	std::cout << std::endl;
	std::cout << "========================================" << std::endl;
	std::cout << title << std::endl;
	std::cout << "========================================" << std::endl;
}

void testValidConstruction()
{
	printSeparator("Test: Valid Construction");
	
	try
	{
		Bureaucrat bob("Bob", 50);
		std::cout << bob << std::endl;
		
		Bureaucrat alice("Alice", 1);
		std::cout << alice << std::endl;
		
		Bureaucrat charlie("Charlie", 150);
		std::cout << charlie << std::endl;
	}
	catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}
}

void testGradeTooHigh()
{
	printSeparator("Test: Grade Too High (0)");
	
	try
	{
		Bureaucrat invalid("Invalid", 0);
		std::cout << invalid << std::endl;
	}
	catch (std::exception& e)
	{
		std::cout << "Exception caught: " << e.what() << std::endl;
	}
}

void testGradeTooLow()
{
	printSeparator("Test: Grade Too Low (151)");
	
	try
	{
		Bureaucrat invalid("Invalid", 151);
		std::cout << invalid << std::endl;
	}
	catch (std::exception& e)
	{
		std::cout << "Exception caught: " << e.what() << std::endl;
	}
}

void testIncrementGrade()
{
	printSeparator("Test: Increment Grade");
	
	try
	{
		Bureaucrat bob("Bob", 5);
		std::cout << "Before increment: " << bob << std::endl;
		bob.incrementGrade();
		std::cout << "After increment:  " << bob << std::endl;
		bob.incrementGrade();
		std::cout << "After increment:  " << bob << std::endl;
	}
	catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}
}

void testDecrementGrade()
{
	printSeparator("Test: Decrement Grade");
	
	try
	{
		Bureaucrat bob("Bob", 148);
		std::cout << "Before decrement: " << bob << std::endl;
		bob.decrementGrade();
		std::cout << "After decrement:  " << bob << std::endl;
		bob.decrementGrade();
		std::cout << "After decrement:  " << bob << std::endl;
	}
	catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}
}

void testIncrementOverflow()
{
	printSeparator("Test: Increment at Grade 1 (Overflow)");
	
	try
	{
		Bureaucrat boss("Boss", 1);
		std::cout << "Before increment: " << boss << std::endl;
		boss.incrementGrade(); // Should throw GradeTooHighException
		std::cout << "After increment:  " << boss << std::endl;
	}
	catch (std::exception& e)
	{
		std::cout << "Exception caught: " << e.what() << std::endl;
	}
}

void testDecrementUnderflow()
{
	printSeparator("Test: Decrement at Grade 150 (Underflow)");
	
	try
	{
		Bureaucrat intern("Intern", 150);
		std::cout << "Before decrement: " << intern << std::endl;
		intern.decrementGrade(); // Should throw GradeTooLowException
		std::cout << "After decrement:  " << intern << std::endl;
	}
	catch (std::exception& e)
	{
		std::cout << "Exception caught: " << e.what() << std::endl;
	}
}

void testCopyConstructor()
{
	printSeparator("Test: Copy Constructor");
	
	try
	{
		Bureaucrat original("Original", 42);
		std::cout << "Original: " << original << std::endl;
		
		Bureaucrat copy(original);
		std::cout << "Copy:     " << copy << std::endl;
		
		// Modify copy to show they are independent
		copy.incrementGrade();
		std::cout << "After incrementing copy:" << std::endl;
		std::cout << "Original: " << original << std::endl;
		std::cout << "Copy:     " << copy << std::endl;
	}
	catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}
}

void testAssignmentOperator()
{
	printSeparator("Test: Assignment Operator");
	
	try
	{
		Bureaucrat bob("Bob", 100);
		Bureaucrat alice("Alice", 50);
		
		std::cout << "Before assignment:" << std::endl;
		std::cout << "Bob:   " << bob << std::endl;
		std::cout << "Alice: " << alice << std::endl;
		
		bob = alice;
		
		std::cout << "After bob = alice:" << std::endl;
		std::cout << "Bob:   " << bob << std::endl;
		std::cout << "Alice: " << alice << std::endl;
		std::cout << "(Note: Bob keeps his name, only grade is copied)" << std::endl;
	}
	catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}
}

void testDefaultConstructor()
{
	printSeparator("Test: Default Constructor");
	
	try
	{
		Bureaucrat defaultBureaucrat;
		std::cout << "Default: " << defaultBureaucrat << std::endl;
	}
	catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}
}

int main()
{
	testValidConstruction();
	testGradeTooHigh();
	testGradeTooLow();
	testIncrementGrade();
	testDecrementGrade();
	testIncrementOverflow();
	testDecrementUnderflow();
	testCopyConstructor();
	testAssignmentOperator();
	testDefaultConstructor();

	printSeparator("All tests completed!");
	
	return 0;
}
