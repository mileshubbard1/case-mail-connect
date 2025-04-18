import { useLocation } from "react-router-dom";
import { useEffect } from "react";
import { Button } from "@/components/ui/button";
import { FileSearch } from "lucide-react";

const NotFound = () => {
  const location = useLocation();

  useEffect(() => {
    console.error(
      "404 Error: User attempted to access non-existent route:",
      location.pathname
    );
  }, [location.pathname]);

  return (
    <div className="min-h-screen flex items-center justify-center bg-gray-50">
      <div className="text-center max-w-md p-8 border rounded-lg shadow-sm bg-white">
        <div className="mx-auto w-16 h-16 bg-legal-light rounded-full flex items-center justify-center mb-4">
          <FileSearch className="h-8 w-8 text-legal-navy" />
        </div>
        <h1 className="text-4xl font-bold mb-2 text-legal-navy">404</h1>
        <p className="text-xl text-legal-slate mb-6">This page could not be found</p>
        <Button asChild className="bg-legal-blue hover:bg-legal-navy">
          <a href="/">Return to Dashboard</a>
        </Button>
      </div>
    </div>
  );
};

export default NotFound;
